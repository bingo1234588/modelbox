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
#ifndef EXTERNAL_DATA_SIMPLE_
#define EXTERNAL_DATA_SIMPLE_
#include <modelbox/flow.h>
#include <queue>

namespace modelbox {
class ExternalDataSimple {
 public:
  ExternalDataSimple(std::shared_ptr<ExternalDataMap> &data_map);
  virtual ~ExternalDataSimple();

  Status PushData(const std::string &port_name, const void *data,
                  const size_t &data_len,
                  const std::map<std::string, std::string> &meta = {});

  Status GetResult(const std::string &port_name,
                   std::shared_ptr<Buffer> &buffer, const int &timeout = 0);

  Status GetResult(const std::string &port_name, std::shared_ptr<void> &data,
                   size_t &len, const int &timeout = 0);

  void Close();

 private:
  std::shared_ptr<ExternalDataMap> data_map_;
  std::map<std::string, std::queue<std::shared_ptr<Buffer>>> buffer_list_map_;
  Status status_;
};

}  // namespace modelbox
#endif
