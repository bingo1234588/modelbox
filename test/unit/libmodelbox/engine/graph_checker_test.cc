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

#include "modelbox/graph_checker.h"

#include "flowunit_mockflowunit/flowunit_mockflowunit.h"
#include "gmock/gmock.h"
#include "graph_conf_mockgraphconf/graph_conf_mockgraphconf.h"
#include "gtest/gtest.h"
#include "mock_driver_ctl.h"
#include "mockflow.h"
#include "modelbox/base/log.h"

using ::testing::_;
namespace modelbox {

class GraphCheckerTest : public testing::Test {
 public:
  GraphCheckerTest() {}

 protected:
  virtual void SetUp() {
    flow_ = std::make_shared<MockFlow>();

    {
      auto mock_desc = GenerateFlowunitDesc("test_0_1", {}, {"Out_1"});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("test_0_2", {}, {"Out_1", "Out_2"});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_3_0", {"In_1", "In_2", "In_3"}, {});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("test_2_0", {"In_1", "In_2"}, {});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("test_1_0", {"In_1"}, {});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_1_1_normal", {"In_1"}, {"Out_1"});
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("test_1_1", {"In_1"}, {"Out_1"});
      mock_desc->SetFlowType(STREAM);
      mock_desc->SetStreamSameCount(true);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("stream_1_1", {"In_1"}, {"Out_1"});
      mock_desc->SetFlowType(STREAM);
      mock_desc->SetStreamSameCount(false);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("condition_1_3", {"In_1"},
                                            {"Out_1", "Out_2", "Out_3"});
      mock_desc->SetConditionType(IF_ELSE);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("condition_1_2", {"In_1"}, {"Out_1", "Out_2"});
      mock_desc->SetConditionType(IF_ELSE);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    // expand and collapse
    {
      auto mock_desc =
          GenerateFlowunitDesc("collapse_1_1", {"In_1"}, {"Out_1"});
      mock_desc->SetOutputType(COLLAPSE);
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("collapse_2_1", {"In_1", "In_2"}, {"Out_1"});
      mock_desc->SetOutputType(COLLAPSE);
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc("expand_1_1", {"In_1"}, {"Out_1"});
      mock_desc->SetOutputType(EXPAND);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("expand_1_2", {"In_1"}, {"Out_1", "Out_2"});
      mock_desc->SetOutputType(EXPAND);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_1_2", {"In_1"}, {"Out_1", "Out_2"});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_1_2_normal", {"In_1"}, {"Out_1", "Out_2"});
      mock_desc->SetFlowType(NORMAL);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_3_1", {"In_1", "In_2", "In_3"}, {"Out_1"});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_2_1", {"In_1", "In_2"}, {"Out_1"});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_loop", {"In_1"}, {"Out_1", "Out_2"});
      mock_desc->SetLoopType(LOOP);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc = GenerateFlowunitDesc(
          "test_loop_invalid", {"In_1", "In_2"}, {"Out_1", "Out_2"});
      mock_desc->SetLoopType(LOOP);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    {
      auto mock_desc =
          GenerateFlowunitDesc("test_1_1_stream", {"In_1"}, {"Out_1"});
      mock_desc->SetFlowType(STREAM);
      auto mock_funcitons = std::make_shared<MockFunctionCollection>();
      flow_->AddFlowUnitDesc(mock_desc, mock_funcitons->GenerateCreateFunc());
    }

    flow_->Init(false);
  }

  virtual void TearDown() {
    auto flowunit_mgr = FlowUnitManager::GetInstance();
    flowunit_mgr->Clear();
    auto device_mgr = DeviceManager::GetInstance();
    device_mgr->Clear();
    auto drivers = Drivers::GetInstance();
    drivers->Clear();
  }

  void BuildGcGraph(std::shared_ptr<Configuration> config,
                    std::shared_ptr<GCGraph> &gcgraph) {
    std::shared_ptr<Drivers> drivers = Drivers::GetInstance();

    auto device_mgr = DeviceManager::GetInstance();
    device_mgr->Initialize(drivers, config);

    auto flowunit_mgr = FlowUnitManager::GetInstance();
    flowunit_mgr->Initialize(drivers, device_mgr, config);

    GraphConfigManager graphconf_mgr = GraphConfigManager::GetInstance();
    graphconf_mgr.Initialize(drivers, config);
    auto graphvizconf = graphconf_mgr.LoadGraphConfig(config);
    gcgraph = graphvizconf->Resolve();
  }

  std::shared_ptr<Graph> InitGraph(std::shared_ptr<Configuration> config) {
    auto device_mgr = DeviceManager::GetInstance();
    auto flowunit_mgr = FlowUnitManager::GetInstance();
    auto graph = std::make_shared<Graph>();
    graph->Initialize(flowunit_mgr, device_mgr, nullptr, config);
    return graph;
  }

  Status BuildGraph(std::shared_ptr<Configuration> config,
                    std::shared_ptr<Graph> &graph) {
    std::shared_ptr<GCGraph> gcgraph;
    BuildGcGraph(config, gcgraph);
    if (!gcgraph) {
      return STATUS_BADCONF;
    }

    graph = InitGraph(config);
    return graph->Build(gcgraph);
  }

  std::shared_ptr<Node> CastNode(std::shared_ptr<NodeBase> node) {
    return std::dynamic_pointer_cast<Node>(node);
  }

  void TestGraph(const std::string &graph, const Status &status) {
    ConfigurationBuilder configbuilder;
    auto config = configbuilder.Build();
    config->SetProperty("graph.format", "graphviz");
    config->SetProperty("graph.graphconf", graph);
    std::shared_ptr<Graph> mb_graph;
    EXPECT_TRUE(BuildGraph(config, mb_graph) == status);
  }

 private:
  std::shared_ptr<MockFlow> flow_;
};

TEST_F(GraphCheckerTest, VirtualNode_NormalFlow) {
  std::string conf_file_value =
      R"(
        digraph demo {
          input1[type=input]
          output1[type=output]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          input1 -> b:In_1
          b:Out_1 -> output1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, VirtualNode_MatchAtVirtualInput) {
  std::string conf_file_value =
      R"(
        digraph demo {
          input1[type=input]
          input2[type=input]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          input1 -> b:In_1
          input2 -> c:In_1
          b:Out_1 -> d:In_1
          c:Out_1 -> d:In_2
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, VirtualNode_MatchMultiInputOutput) {
  std::string conf_file_value =
      R"(
        digraph demo {
          input1[type=input]
          input2[type=input]
          output1[type=output]
          output2[type=output]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          input1 -> b:In_1
          input2 -> c:In_1
          b:Out_1 -> output1
          c:Out_1 -> output2
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}
/*
  a --> b --> d
    |         |
    |         |
    c --------
*/

TEST_F(GraphCheckerTest, SinglePortMatch_SingleOutPortLinkMultiInPort) {
  std::string conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_1 -> c:In_1
          b:Out_1 -> d:In_1
          c:Out_1 -> d:In_2
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

/*
  a --> b --> d
    |       |
    |       |
    c ------
*/

TEST_F(GraphCheckerTest, SinglePortNotMatch_SingleOutPortLinkSingleInPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_1 -> c:In_1
          b:Out_1 -> d:In_1
          c:Out_1 -> d:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

/*
  a --> b --> d
  |           |
  |           |
  c ----------
*/

TEST_F(GraphCheckerTest, MuliPortMatch_MultiOutPortLinkMultiInPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_2, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_2 -> c:In_1
          b:Out_1 -> d:In_1
          c:Out_1 -> d:In_2
          d:Out_1 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

/*
  a --> b --> d
  |         |
  |         |
  c --------
*/

TEST_F(GraphCheckerTest, MuliPortNotMatch_MultiOutPortLinkSingleInPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_2, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_2 -> c:In_1
          b:Out_1 -> d:In_1
          c:Out_1 -> d:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionMatch_OneInPortThreeOutPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_3, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          b:Out_3 -> e:In_1
          c:Out_1 -> f:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ConditionMatch_OutConditionInMultiPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_2, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_2 -> e:In_2
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          d:Out_1 -> e:In_1
          c:Out_1 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ConditionMatch_MutiConditionInSinglePort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          d:Out_1 -> e:In_1
          c:Out_1 -> e:In_1
          c:Out_2 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotMatch_AllOutPortLinkDifferenceInPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_3, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          b:Out_3 -> e:In_1
          c:Out_1 -> f:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotMatch_MultiOutPortLinkInPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_3_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_1 -> d:In_1
          b:Out_2 -> e:In_1
          c:Out_1 -> f:In_1
          d:Out_1 -> f:In_2
          e:Out_1 -> f:In_3
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotMatch_SinglePortConditionNotMatch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_3, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> c:In_2
          b:Out_3 -> d:In_1
          c:Out_1 -> d:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionMatch_SinglePortMatch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_1 -> c:In_2
          b:Out_2 -> d:In_1
          c:Out_1 -> d:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, LoopMatch_LoopSelf) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_loop, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1 
          b:Out_1 -> b:In_1
          b:Out_2 -> c:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, LoopMatch_LoopHasNode) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_loop, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1_normal, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          c:Out_1 -> b:In_1
          b:Out_2 -> d:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, LoopNotMatch_OverHierarchyLink) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=test_loop, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_2_normal, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          c:Out_1 -> b:In_1
          b:Out_2 -> d:In_1
          c:Out_2 -> d:In_2
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ExpandCollapseMatch_NormalFlow) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          d:Out_1 -> e:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseMatch_OnlyExpand) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_OnlyCollapse) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0]
          b[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          c[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ExpandCollapseMatch_OverMatchArch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          f[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          h[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
          g:Out_1 -> h:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_ExpandInMatchArch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          f[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          h[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
          g:Out_1 -> h:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ExpandCollapseMatch_ExpandIsMatchNode) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          c:Out_1 -> e:In_1
          d:Out_1 -> e:In_2
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest,
       ExpandCollapseMatch_MultiOutputExpandDirectConnectCollapse) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> d:In_1
          b:Out_2 -> d:In_2
          d:Out_1 -> e:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseMatch_CollapseIsMatchNode) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1 
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_CollapseIsMatchNode) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_CollapseInMatchArch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest,
       ExpandCollapseNotMatch_CollapseInMatchArch_SinglePathMatch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_OneExpandMultiCollapse) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> f:In_2
          f:Out_1 -> g:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseMatch_MultiArch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0]
          h[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0] 
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          c:Out_1 -> e:In_1
          d:Out_1 -> f:In_1
          e:Out_1 -> g:In_1
          f:Out_1 -> g:In_2
          g:Out_1 -> h:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_OverHierarchyLink_FromOutToIn) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_2, device=cpu, deviceid=0, label="<Out_1>"] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0, label="<In_1> | <Out_1>"] 
          c[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0, label="<In_1> | <In_2> | <Out_1> "]
          d[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0, label="<In_1>"] 
          e[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_2 -> c:In_2
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          d:Out_1 -> e:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ExpandCollapseNotMatch_OverHierarchyLink_FromInToOut) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0, label="<Out_1>"] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0, label="<In_1> | <Out_1>"] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0, label="<In_1> | <In_2> | <Out_1> "]
          d[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0, label="<In_1>"] 
          e[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> e:In_2
          d:Out_1 -> e:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotMatch_OverHierarchyLink_FromOutToIn) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_2, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_2_1, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_2 -> c:In_2
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          c:Out_1 -> e:In_1
          d:Out_1 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotMatch_OverHierarchyLink_FromInToOut) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          c:Out_1 -> e:In_1
          c:Out_2 -> f:In_1
          d:Out_1 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotAddition_MultiConditionLinkSameOut) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          c:Out_1 -> e:In_1
          c:Out_2 -> d:In_1
          d:Out_1 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_BADCONF);
}

TEST_F(GraphCheckerTest, ConditionNotAddition_EndifAndInOtherMultiPort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_2, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          a:Out_2 -> d:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_2
          c:Out_1 -> d:In_2
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ConditionMatch_EndifAndCollapseInOnePort) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          f[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0]
          h[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_1
          c:Out_1 -> d:In_1
          d:Out_1 -> f:In_1
          d:Out_2 -> e:In_1
          e:Out_1 -> f:In_2
          e:Out_2 -> f:In_2
          f:Out_1 -> h:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ConditionAddition_ConditionInExpandCollapse) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_2, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=collapse_2_1, device=cpu, deviceid=0] 
          e[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          b:Out_2 -> d:In_2
          c:Out_1 -> d:In_1
          c:Out_2 -> d:In_1
          d:Out_1 -> e:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, BranchCollapseMatch) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0] 
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=test_2_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          d:Out_1 -> e:In_1
          c:Out_1 -> e:In_2
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, ConditionMatch_SinglePortLinkMultiPortThroughNode) {
  auto conf_file_value =
      R"(
        digraph demo {
          httpserver_sync_receive[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          param_analysis[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0] 
          my_nv_image_decoder[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0] 
          image_resolution_judge[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          color_tranpose_1[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          padding[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          normalize[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          face_detetc_infer[type=flowunit, flowunit=test_1_2, device=cpu, deviceid=0]
          face_detect_post[type=flowunit, flowunit=test_3_1, device=cpu, deviceid=0]
          face_condition[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          g[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]

          httpserver_sync_receive:Out_1 -> param_analysis:In_1
          param_analysis:Out_1 -> my_nv_image_decoder:In_1
          param_analysis:Out_2 -> image_resolution_judge:In_1
          my_nv_image_decoder:Out_1 -> image_resolution_judge:In_1
          image_resolution_judge:Out_1 -> face_detect_post:In_1
          image_resolution_judge:Out_1 -> color_tranpose_1:In_1
          color_tranpose_1:Out_1 -> padding:In_1
          padding:Out_1 -> normalize:In_1
          normalize:Out_1 -> face_detetc_infer:In_1
          face_detetc_infer:Out_1 -> face_detect_post:In_2
          face_detetc_infer:Out_2 -> face_detect_post:In_3
          face_detect_post:Out_1 -> face_condition:In_1
          image_resolution_judge:Out_2 -> face_condition:In_1
          face_condition:Out_1 -> g:In_1
          face_condition:Out_2 -> g:In_1
        }
      )";

  TestGraph(conf_file_value, STATUS_OK);
}

TEST_F(GraphCheckerTest, GetSetMatchNode) {
  auto conf_file_value =
      R"(
        digraph demo {
          a[type=flowunit, flowunit=test_0_1, device=cpu, deviceid=0] 
          b[type=flowunit, flowunit=expand_1_1, device=cpu, deviceid=0] 
          c[type=flowunit, flowunit=condition_1_2, device=cpu, deviceid=0]
          d[type=flowunit, flowunit=test_1_1, device=cpu, deviceid=0]
          e[type=flowunit, flowunit=collapse_1_1, device=cpu, deviceid=0] 
          f[type=flowunit, flowunit=test_1_0, device=cpu, deviceid=0]
          a:Out_1 -> b:In_1
          b:Out_1 -> c:In_1
          c:Out_1 -> d:In_1
          c:Out_2 -> d:In_1
          d:Out_1 -> e:In_1
          e:Out_1 -> f:In_1
        }
      )";
  ConfigurationBuilder configbuilder;
  auto config = configbuilder.Build();
  config->SetProperty("graph.format", "graphviz");
  config->SetProperty("graph.graphconf", conf_file_value);
  std::shared_ptr<Graph> graph;
  EXPECT_TRUE(BuildGraph(config, graph) == STATUS_OK);
  EXPECT_EQ(CastNode(graph->GetNode("a"))->GetMatchNode(), nullptr);
  EXPECT_EQ(CastNode(graph->GetNode("b"))->GetMatchNode(), nullptr);
  EXPECT_EQ(CastNode(graph->GetNode("c"))->GetMatchNode(), nullptr);
  EXPECT_EQ(CastNode(graph->GetNode("d"))->GetMatchNode(), graph->GetNode("c"));
  EXPECT_EQ(CastNode(graph->GetNode("e"))->GetMatchNode(), graph->GetNode("b"));
  EXPECT_EQ(CastNode(graph->GetNode("f"))->GetMatchNode(), nullptr);
}

}  // namespace modelbox