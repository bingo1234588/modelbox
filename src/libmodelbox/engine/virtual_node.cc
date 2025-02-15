/*
 * Copyright 2021 The Modelbox Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "modelbox/virtual_node.h"

#include <modelbox/session.h>
#include <modelbox/session_context.h>
#include <stdint.h>

namespace modelbox {

InputVirtualNode::InputVirtualNode(
    const std::string& device_name, const std::string& device_id,
    std::shared_ptr<DeviceManager> device_manager)
    : device_name_(device_name), device_id_(device_id) {
  queue_size_ = -1;
  priority_ = 0;
  device_mgr_ = device_manager;
}

InputVirtualNode::~InputVirtualNode() {}

Status InputVirtualNode::Init(const std::set<std::string>& input_port_names,
                              const std::set<std::string>& output_port_names,
                              std::shared_ptr<Configuration> config) {
  auto status = NodeBase::Init(input_port_names, output_port_names, config);
  if (status != STATUS_SUCCESS) {
    return status;
  }

  extern_ports_.clear();
  auto ext_queue_size = config->GetUint64("queue_size_external", queue_size_);
  for (auto& output_port_name : output_port_names) {
    auto port = std::make_shared<InPort>(
        output_port_name,
        std::dynamic_pointer_cast<NodeBase>(shared_from_this()), GetPriority(),
        ext_queue_size);
    extern_ports_.emplace_back(port);
  }

  for (auto& port : extern_ports_) {
    port->Init();
  }

  return STATUS_OK;
}

Status InputVirtualNode::Open() { return STATUS_OK; }

std::shared_ptr<Device> InputVirtualNode::GetDevice() {
  if (device_mgr_ == nullptr) {
    MBLOG_ERROR << "device_mgr is nullptr ";
    return nullptr;
  }

  auto device = device_mgr_->CreateDevice(device_name_, device_id_);
  if (device == nullptr) {
    MBLOG_ERROR << "device is nullptr."
                << " device_name: " << device_name_
                << " device_id_: " << device_id_;
    return nullptr;
  }
  return device;
}

Status InputVirtualNode::Run(RunType type) {
  // data from ExternalDataMap has already set inherit info, and match is this
  // input virtual node, we could simply send data to output
  std::unordered_map<std::string, std::vector<std::shared_ptr<Buffer>>>
      ports_data_cache;
  // recv port data
  for (auto& port : extern_ports_) {
    auto& data_cache = ports_data_cache[port->GetName()];
    port->Recv(data_cache, -1);
  }
  // send to output port
  for (auto& port : output_ports_) {
    auto& data_cache = ports_data_cache[port->GetName()];
    if (data_cache.empty()) {
      continue;
    }

    port->Send(data_cache);
  }
  return STATUS_SUCCESS;
}

OutputVirtualNode::OutputVirtualNode(
    const std::string& device_name, const std::string& device_id,
    std::shared_ptr<DeviceManager> device_manager)
    : device_name_(device_name), device_id_(device_id) {
  queue_size_ = -1;
  priority_ = 0;
  device_mgr_ = device_manager;
}

OutputVirtualNode::~OutputVirtualNode() {}

Status OutputVirtualNode::Init(const std::set<std::string>& input_port_names,
                               const std::set<std::string>& output_port_names,
                               std::shared_ptr<Configuration> config) {
  auto status = NodeBase::Init(input_port_names, output_port_names, config);
  if (status != STATUS_SUCCESS) {
    return status;
  }

  auto port_count = GetInputNum();
  if (port_count == 0) {
    port_count = GetExternNum();
  }
  input_match_stream_mgr_ =
      std::make_shared<InputMatchStreamManager>(name_, queue_size_, port_count);
  input_match_stream_mgr_->SetInputBufferInOrder(true);
  input_match_stream_mgr_->SetInputStreamGatherAll(false);
  return STATUS_SUCCESS;
}

Status OutputVirtualNode::Open() { return STATUS_SUCCESS; }

/**
 * @brief remove the data can not send out, in case user debug
 **/
void OutputVirtualNode::EraseInvalidData() {
  for (auto& in_port : input_ports_) {
    auto in_queue = in_port->GetQueue();
    std::shared_ptr<Buffer> buffer;
    while (in_queue->Front(&buffer)) {
      auto index_info = BufferManageView::GetIndexInfo(buffer);
      if (index_info->GetStream()->GetSession()->GetSessionIO() != nullptr) {
        // front data in this port is valid, jump to run
        break;
      }

      in_queue->Pop(&buffer);
    }
  }
}

Status OutputVirtualNode::Run(RunType type) {
  EraseInvalidData();
  std::list<std::shared_ptr<MatchStreamData>> match_stream_data_list;
  auto ret = GenInputMatchStreamData(type, match_stream_data_list);
  if (!ret) {
    MBLOG_ERROR << "OutputVirtualNode generate match stream failed, error "
                << ret;
    return ret;
  }

  if (match_stream_data_list.empty()) {
    return STATUS_SUCCESS;
  }

  for (auto& match_stream_data : match_stream_data_list) {
    auto buffer_count = match_stream_data->GetDataCount();
    if (buffer_count == 0) {
      continue;
    }

    auto stream_data_map = match_stream_data->GetBufferList();
    auto session = match_stream_data->GetSession();

    if (session->IsAbort()) {
      MBLOG_INFO << "session " << session->GetSessionCtx()->GetSessionId()
                 << ", processed over";
      continue;
    }

    // for session end, when all data processed, session will be released
    // automatically
    // send session data to user
    auto io =
        std::dynamic_pointer_cast<ExternalDataMapImpl>(session->GetSessionIO());
    if (io == nullptr) {
      // user release io handle, no need to push output data
      continue;
    }
    OutputBufferList output;
    std::shared_ptr<FlowUnitError> last_error;
    for (auto& port_data : *stream_data_map) {
      auto& port_name = port_data.first;
      auto& data_list = port_data.second;
      std::vector<std::shared_ptr<Buffer>> valid_output;
      for (auto& data : data_list) {
        auto index_info = BufferManageView::GetIndexInfo(data);
        if (index_info->IsEndFlag()) {
          continue;
        }

        if (index_info->IsPlaceholder()) {
          continue;
        }

        if (data->HasError()) {
          last_error = data->GetError();
        }

        valid_output.push_back(data);
      }
      output[port_name] = std::make_shared<BufferList>(valid_output);
    }
    io->PushGraphOutputBuffer(output);
    io->SetLastError(last_error);
  }

  return STATUS_SUCCESS;
}

std::shared_ptr<Device> OutputVirtualNode::GetDevice() {
  if (device_mgr_ == nullptr) {
    MBLOG_ERROR << "device_mgr is nullptr ";
    return nullptr;
  }

  auto device = device_mgr_->CreateDevice(device_name_, device_id_);
  if (device == nullptr) {
    MBLOG_ERROR << "device is nullptr."
                << " device_name: " << device_name_
                << " device_id_: " << device_id_;
    return nullptr;
  }
  return device;
}

SessionUnmatchCache::SessionUnmatchCache(
    const std::set<std::string>& port_names) {
  for (auto& name : port_names) {
    port_end_flag_map_[name] = false;
  }
}

Status SessionUnmatchCache::CacheBuffer(const std::string& port_name,
                                        std::shared_ptr<Buffer> buffer) {
  if (buffer->HasError()) {
    last_error_ = buffer->GetError();
  }

  auto buffer_index = BufferManageView::GetIndexInfo(buffer);

  // cache data
  auto& port_streams = port_streams_map_[port_name];
  auto stream = buffer_index->GetStream();
  port_streams[stream].push_back(buffer);

  // check port end
  if (!buffer_index->IsEndFlag()) {
    return STATUS_OK;
  }

  auto cur_level_buffer = buffer_index;
  while (cur_level_buffer->GetInheritInfo()->GetDeepth() != 0) {
    cur_level_buffer = cur_level_buffer->GetInheritInfo()->GetInheritFrom();
  }

  if (cur_level_buffer->IsEndFlag()) {
    port_end_flag_map_[port_name] = true;
  }

  return STATUS_OK;
}

std::shared_ptr<FlowUnitError> SessionUnmatchCache::GetLastError() {
  return last_error_;
}

Status SessionUnmatchCache::PopCache(OutputBufferList& output_buffer_list) {
  size_t empty_port = 0;
  for (auto& port_streams_item : port_streams_map_) {
    auto& port_name = port_streams_item.first;
    auto& port_streams = port_streams_item.second;
    if (port_streams.empty()) {
      output_buffer_list[port_name] = std::make_shared<BufferList>();
      ++empty_port;
      continue;
    }

    auto first_stream_item = port_streams.begin();
    auto& first_stream_data_list = first_stream_item->second;
    std::vector<std::shared_ptr<Buffer>> valid_data_list;
    for (auto& buffer : first_stream_data_list) {
      auto index = BufferManageView::GetIndexInfo(buffer);
      if (index->IsEndFlag()) {
        continue;
      }

      if (index->IsPlaceholder()) {
        continue;
      }

      valid_data_list.push_back(buffer);
    }
    output_buffer_list[port_name] =
        std::make_shared<BufferList>(valid_data_list);
    port_streams.erase(first_stream_item);
  }

  if (empty_port == port_streams_map_.size()) {
    return STATUS_NODATA;
  }

  return STATUS_CONTINUE;
}

bool SessionUnmatchCache::AllPortStreamEnd() {
  for (auto& port_end_flag : port_end_flag_map_) {
    if (!port_end_flag.second) {
      return false;
    }
  }

  return true;
}

OutputUnmatchVirtualNode::OutputUnmatchVirtualNode(
    const std::string& device_name, const std::string& device_id,
    std::shared_ptr<DeviceManager> device_manager)
    : device_name_(device_name), device_id_(device_id) {
  queue_size_ = -1;
  priority_ = 0;
  device_mgr_ = device_manager;
}

OutputUnmatchVirtualNode::~OutputUnmatchVirtualNode() {}

Status OutputUnmatchVirtualNode::Init(
    const std::set<std::string>& input_port_names,
    const std::set<std::string>& output_port_names,
    std::shared_ptr<Configuration> config) {
  return NodeBase::Init(input_port_names, output_port_names, config);
}

Status OutputUnmatchVirtualNode::Open() { return STATUS_SUCCESS; }

Status OutputUnmatchVirtualNode::Run(RunType type) {
  for (auto& in_port : input_ports_) {
    std::vector<std::shared_ptr<Buffer>> buffers;
    in_port->Recv(buffers, -1);
    for (auto& buffer : buffers) {
      auto buffer_index_info = BufferManageView::GetIndexInfo(buffer);
      auto session = buffer_index_info->GetStream()->GetSession();
      if (session->IsAbort()) {
        continue;
      }
      auto cache_item = session_cache_map_.find(session);
      std::shared_ptr<SessionUnmatchCache> session_cache;
      if (cache_item == session_cache_map_.end()) {
        session_cache = std::make_shared<SessionUnmatchCache>(GetInputNames());
        session_cache_map_[session] = session_cache;
      } else {
        session_cache = cache_item->second;
      }
      session_cache->CacheBuffer(in_port->GetName(), buffer);
    }
  }

  for (auto iter = session_cache_map_.begin();
       iter != session_cache_map_.end();) {
    auto& session = iter->first;
    auto& cache = iter->second;
    auto io =
        std::dynamic_pointer_cast<ExternalDataMapImpl>(session->GetSessionIO());
    if (io != nullptr) {
      OutputBufferList output_buffer_list;
      io->SetLastError(cache->GetLastError());
      while (cache->PopCache(output_buffer_list) != STATUS_NODATA) {
        io->PushGraphOutputBuffer(output_buffer_list);
      }
    }

    if (cache->AllPortStreamEnd() || session->IsAbort()) {
      MBLOG_INFO << "session " << session->GetSessionCtx()->GetSessionId()
                 << ", processed over";
      iter = session_cache_map_.erase(iter);
    } else {
      ++iter;
    }
  }
  return STATUS_SUCCESS;
}

std::shared_ptr<Device> OutputUnmatchVirtualNode::GetDevice() {
  if (device_mgr_ == nullptr) {
    MBLOG_ERROR << "device_mgr is nullptr ";
    return nullptr;
  }

  auto device = device_mgr_->CreateDevice(device_name_, device_id_);
  if (device == nullptr) {
    MBLOG_ERROR << "device is nullptr."
                << " device_name: " << device_name_
                << " device_id_: " << device_id_;
    return nullptr;
  }
  return device;
}

}  // namespace modelbox
