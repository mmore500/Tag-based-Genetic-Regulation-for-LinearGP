/*
 *
 * Inputs:
 *
 * - Numeric
 *   - Unsigned integer values between 0 and [max value]
 * - Operators
 *   - ECHO, NAND, NOT, OR_NOT, AND, OR, AND_NOT, NOR, XOR, EQU
 *
 * Outputs:
 * - Numeric
 * - Error
 * - Wait
 *
 * Scenarios
 *
 * - [ANY-OP]: Error
 * - [NUM][1-INPUT-OP]: Result
 * - [NUM][NUM][2-INPUT-OP]: Result
 * - [NUM][2-INPUT-OP]: Error
 * - [NUM][NUM][NUM]: Error
**/

#include <unordered_set>
#include <functional>
#include <set>
#include <iostream>

#include "base/vector.h"
#include "data/DataFile.h"
#include "tools/Random.h"
#include "tools/set_utils.h"
#include "tools/string_utils.h"

std::unordered_set<std::string> one_input_ops{"ECHO", "NOT"};
// std::unordered_set<std::string> two_input_ops{"NAND","OR_NOT","AND","OR","AND_NOT","NOR","XOR","EQU"};
std::unordered_set<std::string> two_input_ops{"NAND","ORNOT","AND","OR","ANDNOT"};

using operand_t = uint32_t;
using operand_set_t = std::unordered_set<operand_t>;
using operand_pair_set_t = std::set<std::pair<operand_t,operand_t>>;

constexpr operand_t min_numeric=0;
constexpr operand_t max_numeric=1000000000;


constexpr size_t ONE_INPUT_COMPUTATIONS_num_testing_cases=500;
constexpr size_t TWO_INPUT_COMPUTATIONS_num_testing_cases=500;
constexpr size_t ERROR_ONE_ARG_num_testing_cases=200;   // ERROR_NUM_NUM
constexpr size_t ERROR_NO_OP_num_testing_cases=20;   // ERROR_OP_OP (per valid combination)

constexpr size_t ONE_INPUT_COMPUTATIONS_num_training_cases=40;
constexpr size_t TWO_INPUT_COMPUTATIONS_num_training_cases=40;
constexpr size_t ERROR_ONE_ARG_num_training_cases=20;
constexpr size_t ERROR_NO_OP_num_training_cases=20;

struct TestCaseStr {
  std::string input="";
  std::string output="";
  std::string type="";

  TestCaseStr(const std::string & in,
              const std::string & out,
              const std::string & type)
    : input(in), output(out), type(type) { ; }

  void Print(std::ostream & out=std::cout) {
    out << input << "," << output << "," << type;
  }
};

struct Input {
  std::string input_operator="";
  emp::vector<operand_t> operands{0,2};

  Input(const std::string & op, const emp::vector<operand_t> & args)
    : input_operator(op), operands(args) { ; }
};

operand_t ECHO(operand_t a) {
  return a;
}

operand_t NOT(operand_t a) {
  return ~a;
}

operand_t NAND(operand_t a, operand_t b) {
  return ~(a&b);
}

operand_t OR_NOT(operand_t a, operand_t b) {
  return (a|(~b));
}

operand_t AND(operand_t a, operand_t b) {
  return (a&b);
}

operand_t OR(operand_t a, operand_t b) {
  return (a|b);
}

operand_t AND_NOT(operand_t a, operand_t b) {
  return (a&(~b));
}

operand_t NOR(operand_t a, operand_t b) {
  return ~(a|b);
}

operand_t XOR(operand_t a, operand_t b) {
  return (a^b);
}

operand_t EQU(operand_t a, operand_t b) {
  return ~(a^b);
}

operand_t Execute(const Input & input) {
  const std::string & op = input.input_operator;
  emp_assert(emp::Has(one_input_ops, op) || emp::Has(two_input_ops, op));
  emp_assert((emp::Has(one_input_ops, op) && input.operands.size() > 0)
              || (emp::Has(two_input_ops, op) && input.operands.size() >= 2));
  if      (op == "ECHO")    { return ECHO(input.operands[0]); }
  else if (op == "NAND")    { return NAND(input.operands[0], input.operands[1]); }
  else if (op == "NOT")     { return NOT(input.operands[0]); }
  else if (op == "ORNOT")  { return OR_NOT(input.operands[0], input.operands[1]); }
  else if (op == "AND")     { return AND(input.operands[0], input.operands[1]); }
  else if (op == "OR")      { return OR(input.operands[0], input.operands[1]); }
  else if (op == "ANDNOT") { return AND_NOT(input.operands[0], input.operands[1]); }
  else if (op == "NOR")     { return NOR(input.operands[0], input.operands[1]); }
  else if (op == "XOR")     { return XOR(input.operands[0], input.operands[1]); }
  else if (op == "EQU")     { return EQU(input.operands[0], input.operands[1]); }
  else { emp_assert(false); return 0; }
}

TestCaseStr GenOPErrorTestCase(const std::string & op) {
  return {"OP:" + op, "ERROR", "ERROR_"+op};
}

TestCaseStr GenAllNumErrorTestCase(operand_t a, operand_t b, operand_t c) {
  return {"NUM:" + emp::to_string(a) + ";NUM:" + emp::to_string(b) + ";NUM:" + emp::to_string(c),  // Input
          "ERROR",                                      // Output
          "ERROR_ALL_NUM"};                             // Type
}

TestCaseStr GenOneArgErrorTestCase(const std::string & op_a,
                                   operand_t num)
{
  return {
    "NUM:" + emp::to_string(num) + ";OP:" + op_a,
    "ERROR",
    "ERROR_NUM_"+op_a
  };
}

TestCaseStr GenOneInputValidTestCase(const std::string & op, operand_t a) {
  return {
    "NUM:" + emp::to_string(a) + ";OP:" + op,       // Input
    emp::to_string(Execute({op, {a}})),             // Output
    op                                              // Type
  };
}

TestCaseStr GenTwoInputValidTestCase(const std::string & op,
                                     const std::pair<operand_t, operand_t> operands)
{
  return {
    "NUM:" + emp::to_string(operands.first) + ";NUM:" + emp::to_string(operands.second) + ";OP:" + op , // Input
    emp::to_string(Execute({op, {operands.first, operands.second}})),                  // Output
    op                                                                                 // Type
  };
}

operand_set_t GenUniqueValues(emp::Random & rand,
                              size_t num_vals,
                              const operand_set_t & unique_from=operand_set_t())
{
  operand_set_t vals;
  while (vals.size() < num_vals) {
    operand_t val = rand.GetUInt(min_numeric, max_numeric);
    if (emp::Has(unique_from, val)) continue;
    vals.emplace(val);
  }
  return vals;
}


operand_pair_set_t GenUniquePairs(emp::Random & rand,
                                  size_t num_pairs,
                                  const operand_pair_set_t & unique_from=operand_pair_set_t())
{
  operand_pair_set_t pairs;
  while (pairs.size() < num_pairs) {
    std::pair<operand_t,operand_t> pair = {rand.GetUInt(min_numeric, max_numeric),
                                           rand.GetUInt(min_numeric, max_numeric)};
    if (emp::Has(unique_from, pair)) continue;
    pairs.emplace(pair);
  }
  return pairs;
}

int main() {
  emp::Random random(2);

  std::unordered_set<std::string> all_input_ops(one_input_ops);
  for (const std::string & op : two_input_ops) {
    all_input_ops.emplace(op);
  }

  emp::vector<TestCaseStr> testing_set;
  emp::vector<TestCaseStr> training_set;

  auto testing_datafile = emp::MakeContainerDataFile(
    std::function<emp::vector<TestCaseStr>()> {
      [&testing_set]() { return testing_set; }
    },
    "testing_set.csv"
  );
  auto training_datafile = emp::MakeContainerDataFile(
    std::function<emp::vector<TestCaseStr>()> {
      [&training_set]() { return training_set; }
    },
    "training_set.csv"
  );

  // (1) Generate all [OP] tests
  for (const std::string & op : all_input_ops) {
    testing_set.emplace_back(GenOPErrorTestCase(op));
  }
  for (const std::string & op : all_input_ops) {
    training_set.emplace_back(GenOPErrorTestCase(op));
  }

  // (2) Generate all ([NUM][1-INPUT-OP]: Result) tests
  for (const std::string & op : one_input_ops) {
    auto testing_vals = GenUniqueValues(random, ONE_INPUT_COMPUTATIONS_num_testing_cases);
    auto training_vals = GenUniqueValues(random, ONE_INPUT_COMPUTATIONS_num_training_cases, testing_vals);
    for (const auto & val : testing_vals) {
      testing_set.emplace_back(GenOneInputValidTestCase(op, val));
    }
    for (const auto & val : training_vals) {
      training_set.emplace_back(GenOneInputValidTestCase(op, val));
    }
  }

  // (3) Generate all ([NUM][NUM][2-INPUT-OP]: Result) tests
  for (const std::string & op : two_input_ops) {
    auto testing_pairs = GenUniquePairs(random, TWO_INPUT_COMPUTATIONS_num_testing_cases);
    auto training_pairs = GenUniquePairs(random, TWO_INPUT_COMPUTATIONS_num_training_cases, testing_pairs);
    for (const auto & pair : testing_pairs) {
      testing_set.emplace_back(GenTwoInputValidTestCase(op, pair));
    }
    for (const auto & pair : training_pairs) {
      training_set.emplace_back(GenTwoInputValidTestCase(op, pair));
    }
  }

  // (4) Generate all ([NUM][2-INPUT-OP]: Error) tests
  for (const std::string & op : two_input_ops) {
    auto testing_vals = GenUniqueValues(random, ERROR_ONE_ARG_num_testing_cases);
    auto training_vals = GenUniqueValues(random, ERROR_ONE_ARG_num_training_cases, testing_vals);
    for (const auto & val : testing_vals) {
      testing_set.emplace_back(GenOneArgErrorTestCase(op, val));
    }
    for (const auto & val : training_vals) {
      training_set.emplace_back(GenOneArgErrorTestCase(op, val));
    }
  }

  // (5) [NUM][NUM][NUM]: Error
  // ERROR_NO_OP_num_training_cases
  // ERROR_NO_OP_num_testing_cases
  std::function<emp::vector<operand_t>(const std::unordered_set<operand_t> &)>
    to_vec = [](const std::unordered_set<operand_t> & in) {
      return emp::vector<operand_t>(in.begin(), in.end());
    };


  // ... such ugly code ...
  auto testing_pos0 = GenUniqueValues(random, ERROR_NO_OP_num_testing_cases);
  auto testing_pos0_vec = to_vec(testing_pos0);
  auto testing_pos1 = GenUniqueValues(random, ERROR_NO_OP_num_testing_cases);
  auto testing_pos1_vec = to_vec(testing_pos1);
  auto testing_pos2 = GenUniqueValues(random, ERROR_NO_OP_num_testing_cases);
  auto testing_pos2_vec = to_vec(testing_pos2);

  auto training_pos0 = GenUniqueValues(random, ERROR_NO_OP_num_training_cases, testing_pos0);
  auto training_pos0_vec = to_vec(training_pos0);
  auto training_pos1 = GenUniqueValues(random, ERROR_NO_OP_num_training_cases, testing_pos1);
  auto training_pos1_vec = to_vec(training_pos1);
  auto training_pos2 = GenUniqueValues(random, ERROR_NO_OP_num_training_cases, testing_pos2);
  auto training_pos2_vec = to_vec(training_pos2);

  for (size_t i = 0; i < ERROR_NO_OP_num_testing_cases; ++i) {
    testing_set.emplace_back(GenAllNumErrorTestCase(testing_pos0_vec[i],testing_pos1_vec[i],testing_pos2_vec[i]));
  }

  for (size_t i = 0; i < ERROR_NO_OP_num_training_cases; ++i) {
    training_set.emplace_back(GenAllNumErrorTestCase(training_pos0_vec[i],training_pos1_vec[i],training_pos2_vec[i]));
  }


  std::function<std::string(TestCaseStr)> get_input = [](TestCaseStr test) { return test.input; };
  std::function<std::string(TestCaseStr)> get_output = [](TestCaseStr test) { return test.output; };
  std::function<std::string(TestCaseStr)> get_type = [](TestCaseStr test) { return test.type; };

  testing_datafile.AddContainerFun(get_input, "input", "test case input");
  testing_datafile.AddContainerFun(get_output, "output", "test case output");
  testing_datafile.AddContainerFun(get_type, "type", "test case type");
  testing_datafile.PrintHeaderKeys();
  testing_datafile.Update();

  training_datafile.AddContainerFun(get_input, "input", "test case input");
  training_datafile.AddContainerFun(get_output, "output", "test case output");
  training_datafile.AddContainerFun(get_type, "type", "test case type");
  training_datafile.PrintHeaderKeys();
  training_datafile.Update();

  return 0;
}