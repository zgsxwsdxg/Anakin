/* Copyright (c) 2018 Baidu, Inc. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0
   
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. 
*/

#ifndef ANAKIN_FRAMEWORK_LITE_CODE_GEN_BASE_H
#define ANAKIN_FRAMEWORK_LITE_CODE_GEN_BASE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "framework/graph/graph.h"

namespace anakin {

namespace lite {

/**
 * \brief Node information for generating executor
 */
struct NodeInfo {
	std::string name;				// node name
	std::string op_name;			// op name 
	std::vector<std::string> ins;	// input edge name
	std::vector<std::string> outs;	// output edge name
};


/**
 * \brief Edge information for generating edge tensors.
 */
struct EdgeInfo {
	std::string name;	 			// edge name 
	std::vector<int> valid_shape; 	// edge valid shape
	std::vector<int> real_shape;	// edge real shape
	bool is_shared{false}; 			// if the edge is shared by others
	std::string share_from{""}; 	// if the edge is_shared(true), share_from will hold the target edge name.
};

/**  
 *  \brief class for target language code generator.
 *
 *  The class CodeGenBase hold base information for running model.
 *  There exists several base info:
 *  	1. Operatoin name in execution order.
 *  	2. All the tensor model needs and share info between those tensors.
 *  	3. Model weights
 */
template<typename Ttype, DataType Dtype, Precision Ptype>
class CodeGenBase {
public:
	CodeGenBase() {}
	virtual ~CodeGenBase(){}

	/**
	 *  \biref initialize graph msg
	 */
	bool extract_graph(Graph<Ttype, Dtype, Ptype>& graph);

private:
	/**
	 * \brief analyse the memory reuse info
	 */
	bool init_memory_info();

	/**
	 * \brief parsing parameter of graph
	 */
	virtual bool parser_param()=0;

protected:
	Graph<Ttype, Dtype, Ptype> _graph;
	std::vector<std::string> _exec_node_order; /// running order of operation's name
	std::unordered_map<std::string, NodeInfo> _graph_node_map;
	/// graph base arch
	std::unordered_map<std::string, EdgeInfo> _tensor_map;
};

} /* namespace lite */

} /* namespace anakin */

#endif

