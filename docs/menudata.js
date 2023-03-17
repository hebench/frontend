/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var menudata={children:[
{text:"Main Page",url:"index.html"},
{text:"HEBench Supported Benchmark Categories",url:"category_overview.html",children:[
{text:"Latency",url:"category_latency.html"},
{text:"Offline",url:"category_offline.html"},
{text:"Ordering of Offline Results",url:"results_order.html"}]},
{text:"HEBench Supported Workloads",url:"tests_overview.html",children:[
{text:"Generic Workload",url:"generic_workload.html",children:[
{text:"Generic Workload Example",url:"generic_wl_example.html"}]},
{text:"Dot Product",url:"dot_product.html"},
{text:"Elementwise Addition",url:"elementwise_add.html"},
{text:"Elementwise Multiplication",url:"elementwise_mult.html"},
{text:"Logistic Regression Inference",url:"logistic_regression.html"},
{text:"Matrix Multiplication",url:"matrix_multiplication.html"},
{text:"Simple Set Intersection",url:"simple_set_intersection.html"}]},
{text:"Backend Overview",url:"backend_overview.html",children:[
{text:"API Bridge Overview",url:"APIBridge_overview.html"},
{text:"API Bridge Function Pipeline",url:"function_pipeline_chart.html"},
{text:"C++ Wrapper Overview",url:"CPP_overview.html"},
{text:"Tutorial: Backend Creation using C++ Wrapper",url:"simple_cpp_example.html",children:[
{text:"Tutorial using PALISADE",url:"simple_cpp_example_palisade.html",children:[
{text:"Preparation",url:"be_tutorial_preparation_palisade.html"},
{text:"Engine Initialization and Benchmark Description",url:"be_tutorial_init_palisade.html"},
{text:"Benchmark Implementation",url:"be_tutorial_impl_palisade.html"},
{text:"File Reference",url:"be_tutorial_files_palisade.html"}]},
{text:"Tutorial using Microsoft SEAL",url:"simple_cpp_example_seal.html",children:[
{text:"Preparation",url:"be_tutorial_preparation_seal.html"},
{text:"Engine Initialization and Benchmark Description",url:"be_tutorial_init_seal.html"},
{text:"Benchmark Implementation",url:"be_tutorial_impl_seal.html"},
{text:"File Reference",url:"be_tutorial_files_seal.html"}]}]}]},
{text:"Frontend Overview",url:"frontend_overview.html",children:[
{text:"Test Harness Overview",url:"test_harness_overview.html",children:[
{text:"Test Harness User Guide",url:"test_harness_usage_guide.html"},
{text:"Benchmark Configuration File Reference",url:"config_file_reference.html"},
{text:"External Dataset Loader",url:"dataset_loader_overview.html",children:[
{text:"CSV Format Reference",url:"dataset_csv_reference.html"}]}]},
{text:"Report Compiler Overview",url:"report_compiler_overview.html",children:[
{text:"Report Compiler User Guide",url:"report_compiler_usage_guide.html"},
{text:"Statistics and Summary Output Format",url:"stats_n_summary_format.html"},
{text:"Run Overview Output Format",url:"overview_file_format.html"}]},
{text:"Working with Unsupported Workloads",url:"intro_workload_addition.html",children:[
{text:"Custom Workload to Benchmark Unsupported Workloads",url:"generic_workload.html"},
{text:"Add New Workload to Test Harness",url:"extend_test_harness.html",children:[
{text:"Add latency Category to Workload",url:"extend_test_harness_l.html"},
{text:"Add offline Category to Workload",url:"extend_test_harness_o.html"}]}]}]},
{text:"Namespaces",url:"namespaces.html",children:[
{text:"Namespace List",url:"namespaces.html"},
{text:"Namespace Members",url:"namespacemembers.html",children:[
{text:"All",url:"namespacemembers.html",children:[
{text:"a",url:"namespacemembers.html#index_a"},
{text:"c",url:"namespacemembers.html#index_c"},
{text:"d",url:"namespacemembers.html#index_d"},
{text:"e",url:"namespacemembers.html#index_e"},
{text:"f",url:"namespacemembers.html#index_f"},
{text:"g",url:"namespacemembers.html#index_g"},
{text:"h",url:"namespacemembers.html#index_h"},
{text:"i",url:"namespacemembers.html#index_i"},
{text:"l",url:"namespacemembers.html#index_l"},
{text:"m",url:"namespacemembers.html#index_m"},
{text:"n",url:"namespacemembers.html#index_n"},
{text:"o",url:"namespacemembers.html#index_o"},
{text:"p",url:"namespacemembers.html#index_p"},
{text:"s",url:"namespacemembers.html#index_s"},
{text:"t",url:"namespacemembers.html#index_t"},
{text:"u",url:"namespacemembers.html#index_u"},
{text:"v",url:"namespacemembers.html#index_v"},
{text:"w",url:"namespacemembers.html#index_w"}]},
{text:"Functions",url:"namespacemembers_func.html",children:[
{text:"a",url:"namespacemembers_func.html#index_a"},
{text:"c",url:"namespacemembers_func.html#index_c"},
{text:"d",url:"namespacemembers_func.html#index_d"},
{text:"e",url:"namespacemembers_func.html#index_e"},
{text:"f",url:"namespacemembers_func.html#index_f"},
{text:"g",url:"namespacemembers_func.html#index_g"},
{text:"h",url:"namespacemembers_func.html#index_h"},
{text:"i",url:"namespacemembers_func.html#index_i"},
{text:"l",url:"namespacemembers_func.html#index_l"},
{text:"o",url:"namespacemembers_func.html#index_o"},
{text:"p",url:"namespacemembers_func.html#index_p"},
{text:"s",url:"namespacemembers_func.html#index_s"},
{text:"t",url:"namespacemembers_func.html#index_t"},
{text:"v",url:"namespacemembers_func.html#index_v"}]},
{text:"Variables",url:"namespacemembers_vars.html"},
{text:"Typedefs",url:"namespacemembers_type.html"},
{text:"Enumerations",url:"namespacemembers_enum.html"},
{text:"Enumerator",url:"namespacemembers_eval.html"}]}]},
{text:"Classes",url:"annotated.html",children:[
{text:"Class List",url:"annotated.html"},
{text:"Class Index",url:"classes.html"},
{text:"Class Hierarchy",url:"inherits.html"},
{text:"Class Members",url:"functions.html",children:[
{text:"All",url:"functions.html",children:[
{text:"a",url:"functions.html#index_a"},
{text:"b",url:"functions_b.html#index_b"},
{text:"c",url:"functions_c.html#index_c"},
{text:"d",url:"functions_d.html#index_d"},
{text:"e",url:"functions_e.html#index_e"},
{text:"f",url:"functions_f.html#index_f"},
{text:"g",url:"functions_g.html#index_g"},
{text:"h",url:"functions_h.html#index_h"},
{text:"i",url:"functions_i.html#index_i"},
{text:"k",url:"functions_k.html#index_k"},
{text:"l",url:"functions_l.html#index_l"},
{text:"m",url:"functions_m.html#index_m"},
{text:"n",url:"functions_n.html#index_n"},
{text:"o",url:"functions_o.html#index_o"},
{text:"p",url:"functions_p.html#index_p"},
{text:"r",url:"functions_r.html#index_r"},
{text:"s",url:"functions_s.html#index_s"},
{text:"t",url:"functions_t.html#index_t"},
{text:"u",url:"functions_u.html#index_u"},
{text:"v",url:"functions_v.html#index_v"},
{text:"w",url:"functions_w.html#index_w"},
{text:"~",url:"functions_~.html#index__7E"}]},
{text:"Functions",url:"functions_func.html",children:[
{text:"a",url:"functions_func.html#index_a"},
{text:"b",url:"functions_func_b.html#index_b"},
{text:"c",url:"functions_func_c.html#index_c"},
{text:"d",url:"functions_func_d.html#index_d"},
{text:"e",url:"functions_func_e.html#index_e"},
{text:"f",url:"functions_func_f.html#index_f"},
{text:"g",url:"functions_func_g.html#index_g"},
{text:"h",url:"functions_func_h.html#index_h"},
{text:"i",url:"functions_func_i.html#index_i"},
{text:"k",url:"functions_func_k.html#index_k"},
{text:"l",url:"functions_func_l.html#index_l"},
{text:"m",url:"functions_func_m.html#index_m"},
{text:"n",url:"functions_func_n.html#index_n"},
{text:"o",url:"functions_func_o.html#index_o"},
{text:"p",url:"functions_func_p.html#index_p"},
{text:"r",url:"functions_func_r.html#index_r"},
{text:"s",url:"functions_func_s.html#index_s"},
{text:"t",url:"functions_func_t.html#index_t"},
{text:"v",url:"functions_func_v.html#index_v"},
{text:"~",url:"functions_func_~.html#index__7E"}]},
{text:"Variables",url:"functions_vars.html",children:[
{text:"a",url:"functions_vars.html#index_a"},
{text:"b",url:"functions_vars_b.html#index_b"},
{text:"c",url:"functions_vars_c.html#index_c"},
{text:"d",url:"functions_vars_d.html#index_d"},
{text:"e",url:"functions_vars_e.html#index_e"},
{text:"f",url:"functions_vars_f.html#index_f"},
{text:"h",url:"functions_vars_h.html#index_h"},
{text:"i",url:"functions_vars_i.html#index_i"},
{text:"m",url:"functions_vars_m.html#index_m"},
{text:"n",url:"functions_vars_n.html#index_n"},
{text:"o",url:"functions_vars_o.html#index_o"},
{text:"p",url:"functions_vars_p.html#index_p"},
{text:"r",url:"functions_vars_r.html#index_r"},
{text:"s",url:"functions_vars_s.html#index_s"},
{text:"t",url:"functions_vars_t.html#index_t"},
{text:"v",url:"functions_vars_v.html#index_v"},
{text:"w",url:"functions_vars_w.html#index_w"}]},
{text:"Typedefs",url:"functions_type.html"},
{text:"Enumerations",url:"functions_enum.html"},
{text:"Enumerator",url:"functions_eval.html"},
{text:"Related Functions",url:"functions_rela.html"}]}]},
{text:"Files",url:"files.html",children:[
{text:"File List",url:"files.html"},
{text:"File Members",url:"globals.html",children:[
{text:"All",url:"globals.html",children:[
{text:"_",url:"globals.html#index__5F"},
{text:"c",url:"globals.html#index_c"},
{text:"d",url:"globals.html#index_d"},
{text:"f",url:"globals.html#index_f"},
{text:"g",url:"globals.html#index_g"},
{text:"h",url:"globals.html#index_h"},
{text:"i",url:"globals.html#index_i"},
{text:"l",url:"globals.html#index_l"},
{text:"m",url:"globals.html#index_m"},
{text:"n",url:"globals.html#index_n"},
{text:"p",url:"globals.html#index_p"},
{text:"s",url:"globals.html#index_s"},
{text:"t",url:"globals.html#index_t"}]},
{text:"Functions",url:"globals_func.html"},
{text:"Variables",url:"globals_vars.html"},
{text:"Enumerations",url:"globals_enum.html"},
{text:"Enumerator",url:"globals_eval.html"},
{text:"Macros",url:"globals_defs.html"}]}]},
{text:"Glossary",url:"glossary.html"}]}
