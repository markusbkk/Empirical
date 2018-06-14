/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  signalgp_utils.h
 *  @brief Helper functions for working with SignalGP virtual hardware/programs.
 *  @todo Mutator class
 *  @todo tests
 */

#ifndef EMP_SIGNALGP_UTILS_H
#define EMP_SIGNALGP_UTILS_H

#include <unordered_set>
#include <string>
#include <functional>
#include <algorithm>

#include "base/errors.h"
#include "hardware/EventDrivenGP.h"
#include "tools/BitSet.h"
#include "tools/math.h"
#include "tools/Random.h"
#include "tools/random_utils.h"

namespace emp {

  /// Generate one random SignalGP tag (BitSet<TAG_WIDTH>). Given a vector of other tags (unique_from), this 
  /// function will guarantee the tag generated is unique with respect to those tags. 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param unique_from - Other tags that the tag being generated should be unique with respect to. 
  template<size_t TAG_WIDTH>
  BitSet<TAG_WIDTH> GenRandSignalGPTag(emp::Random & rnd, const emp::vector<BitSet<TAG_WIDTH>> & unique_from=emp::vector<BitSet<TAG_WIDTH>>()) {
    using tag_t = BitSet<TAG_WIDTH>;  
    emp_assert(unique_from.size() < emp::Pow2(TAG_WIDTH), "Tag width is not large enough to be able to guarantee requested number of unique tags"); 
    tag_t new_tag(rnd, 0.5); // Make a random tag.
    bool guarantee_unique = (bool)unique_from.size();
    while (guarantee_unique) {
      guarantee_unique = false;
      for (size_t i = 0; i < unique_from.size(); ++i) {
        if (unique_from[i] == new_tag) {
          guarantee_unique = true;
          new_tag.Randomize(rnd);
          break;
        }
      }
    }
    return new_tag;
  }


  /// Generate 'count' number of random SignalGP tags (BitSet<TAG_WIDTH>). 
  /// Given a vector of other tags (unique_from), this function will guarantee the tags generated are unique with respect to those tags. 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param count - How many tags should be generated? 
  /// @param guarantee_unique - Should generated tags be guaranteed to be unique from each other and from tags in 'unique_from'?
  /// @param unique_from - Other tags that the tag being generated should be unique with respect to. Only used if 'guarantee_unique' is true. 
  template<size_t TAG_WIDTH>
  emp::vector<BitSet<TAG_WIDTH>> GenRandSignalGPTags(emp::Random & rnd, size_t count, bool guarantee_unique=false, 
                                                     const emp::vector<BitSet<TAG_WIDTH>> & unique_from=emp::vector<BitSet<TAG_WIDTH>>()) {
    using tag_t = BitSet<TAG_WIDTH>;  
    emp_assert(!guarantee_unique || (unique_from.size()+count <= emp::Pow2(TAG_WIDTH)), "Tag width is not large enough to be able to guarantee requested number of unique tags"); 

    std::unordered_set<uint32_t> uset; // Used to ensure all generated tags are unique.
    emp::vector<tag_t> new_tags;
    for (size_t i = 0; i < unique_from.size(); ++i) uset.emplace(unique_from[i].GetUInt(0));
    for (size_t i = 0; i < count; ++i) {
      new_tags.emplace_back(tag_t());
      new_tags[i].Randomize(rnd);
      if (guarantee_unique) {
        uint32_t tag_int = new_tags[i].GetUInt(0);
        while (true) {
          if (!emp::Has(uset, tag_int)) {
            uset.emplace(tag_int);
            break;
          } else {
            new_tags[i].Randomize(rnd);
            tag_int = new_tags[i].GetUInt(0);
          }
        }
      }
    }
    return new_tags;
  }

  /// Generate a random SignalGP instruction (templated off of tag width). 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param inst_lib - Instruction library used to generate the instruction (instruction will be valid within instruction library)
  /// @param min_arg_val - Mininum value for an instruction argument.
  /// @param max_arg_val - Maximum value for an instruction argument.
  template<size_t TAG_WIDTH> 
  typename EventDrivenGP_AW<TAG_WIDTH>::Instruction GenRandSignalGPInst(emp::Random & rnd, const emp::InstLib<EventDrivenGP_AW<TAG_WIDTH>> & inst_lib, int min_arg_val=0, int max_arg_val=15) {
    emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
    emp_assert(min_arg_val < max_arg_val, "Minimum argument value must be less than maximum argument value to generate a number between the two.");
    using inst_t = typename EventDrivenGP_AW<TAG_WIDTH>::Instruction;
    using tag_t = BitSet<TAG_WIDTH>;
    return inst_t(rnd.GetUInt(inst_lib.GetSize()),
                  rnd.GetInt(min_arg_val, max_arg_val+1),
                  rnd.GetInt(min_arg_val, max_arg_val+1),
                  rnd.GetInt(min_arg_val, max_arg_val+1),
                  tag_t(rnd, 0.5));       
  }

  /// Generate a random SignalGP function (templated off of tag width). 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param inst_lib - Instruction library used to generate the function (instruction will be valid within instruction library)
  /// @param min_inst_cnt - Minimum number of instructions in generated function.
  /// @param max_inst_cnt - Maximum number of instructions in generated function.
  /// @param min_arg_val - Mininum value for an instruction argument.
  /// @param max_arg_val - Maximum value for an instruction argument.
  template<size_t TAG_WIDTH>
  typename EventDrivenGP_AW<TAG_WIDTH>::Function GenRandSignalGPFunction(emp::Random & rnd, const emp::InstLib<EventDrivenGP_AW<TAG_WIDTH>> & inst_lib, 
                                                                            size_t min_inst_cnt=1, size_t max_inst_cnt=32, 
                                                                            int min_arg_val=0, int max_arg_val=15) {
    emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
    using fun_t = typename EventDrivenGP_AW<TAG_WIDTH>::Function;
    size_t inst_cnt = rnd.GetUInt(min_inst_cnt, max_inst_cnt+1);
    fun_t new_fun(emp::GenRandSignalGPTag<TAG_WIDTH>(rnd));
    for (size_t i = 0; i < inst_cnt; ++i) new_fun.PushInst(emp::GenRandSignalGPInst(rnd, inst_lib, min_arg_val, max_arg_val));
    return new_fun;
  }

  /// Generate a random SignalGP program (templated off of tag width). 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param inst_lib - Instruction library used to generate the program. 
  /// @param min_func_cnt - Mininum number of functions in generated program.
  /// @param max_func_cnt - Maximum number of functions in generated program.
  /// @param min_inst_cnt - Minimum number of instructions per function.
  /// @param max_inst_cnt - Maximum number of instructions per function.
  /// @param min_arg_val - Mininum value for an instruction argument.
  /// @param max_arg_val - Maximum value for an instruction argument.
  template<size_t TAG_WIDTH>
  typename EventDrivenGP_AW<TAG_WIDTH>::Program GenRandSignalGPProgram(emp::Random & rnd, const emp::InstLib<EventDrivenGP_AW<TAG_WIDTH>> & inst_lib, 
                                                                       size_t min_func_cnt=1, size_t max_func_cnt=16,
                                                                       size_t min_fun_len=1, size_t max_fun_len=32, 
                                                                       int min_arg_val=0, int max_arg_val=15) {
    emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
    using program_t = typename EventDrivenGP_AW<TAG_WIDTH>::Program;
    program_t program(&inst_lib);
    size_t fun_cnt = rnd.GetUInt(min_func_cnt, max_func_cnt+1);
    for (size_t f = 0; f < fun_cnt; ++f) 
      program.PushFunction(emp::GenRandSignalGPFunction(rnd, inst_lib, min_fun_len, max_fun_len, min_arg_val, max_arg_val));
    return program;  
  }

  /// The SignalGPMutator struct... 
  /// NOTE: could use some feedback on this!
  ///  - Not loving the inconsistency between rates and constraints at the moment. 
  template<size_t TAG_WIDTH>
  class SignalGPMutator {
  public:
    // Generally useful aliases
    // - Hardware aliases
    using hardware_t = EventDrivenGP_AW<TAG_WIDTH>;
    using program_t = typename hardware_t::program_t;
    using tag_t = typename hardware_t::affinity_t;
    using inst_lib_t = typename hardware_t::inst_lib_t;
    using inst_t = typename hardware_t::inst_t;
    using fun_t = typename hardware_t::Function; 
    using mutator_fun_t = std::function<size_t(program_t &, emp::Random &)>;
    
    /// Tags can belong to two types of programmatic entities in SignalGP: functions and instructions.
    enum TagType { FUNCTION=0, INSTRUCTION=1 }; 

    /// Struct used to define a mutation rate.
    struct MutatorParamDef {
      std::string name;
      double param;
      std::string desc;

      MutatorParamDef(const std::string & _n, double _p, const std::string & _d)
        : name(_n), param(_p), desc(_d) { ; }
      MutatorParamDef(const MutatorParamDef &) = default;
    };

    /// Struct used to define a mutation operator. 
    struct MutatorDef {
      std::string name;
      mutator_fun_t mutator;
      std::string desc;
      size_t last_mut_cnt;

      MutatorDef(const std::string & _n, const mutator_fun_t & _fun, const std::string & _d) 
        : name(_n), mutator(_fun), desc(_d), last_mut_cnt(0) { ; }
    };
  
  protected:
    emp::vector<MutatorParamDef> param_lib;
    std::map<std::string, size_t> param_name_map;

    emp::vector<MutatorDef> mutator_lib;
    std::map<std::string, size_t> mutator_name_map;

    // -- General SignalGP program constraints --
    // Program constraints
    size_t prog_min_func_cnt;   ///< Minimum number of functions mutations are allowed to reduce a SignalGP program to.
    size_t prog_max_func_cnt;   ///< Maximum number of functions a mutated SignalGP program can grow to. 
    size_t prog_min_func_len;   ///< Minimum number of instructions a SignalGP function can shrink to.
    size_t prog_max_func_len;   ///< Maximum number of instructions a SignalGP function can grow to.
    size_t prog_max_total_len;  ///< Maximum number of *total* instructions a SignalGP program can grow to. 

    int prog_min_arg_val;       ///< Minimum argument value a SignalGP instruction can mutate to. 
    int prog_max_arg_val;       ///< Maximum argument value a SignalGP instruction can mutate to.       


    // IDs to default mutate rates. 
    size_t arg_sub__per_arg__id;      ///< Rate ID for: Rate to apply substitutions to instruction arguments.
    
    size_t inst_sub__per_inst__id;    ///< Rate ID for: Per-instruction rate to apply instruction substitutions. 
    size_t inst_ins__per_inst__id;    ///< Rate ID for: Per-instruction rate to apply instruction insertions.
    size_t inst_del__per_inst__id;    ///< Rate ID for: Per-instruction rate to apply instruction deletions.
    
    size_t slip__per_func__id;        ///< Rate ID for: Per-function rate to apply slip-mutations.
    size_t func_dup__per_func__id;    ///< Rate ID for: Per-function rate to apply function duplications.
    size_t func_del__per_func__id;    ///< Rate ID for: Per-function rate to apply function deletions.

    size_t tag_bit_flip__per_bit__id; ///< Rate ID for: Per-bit rate to apply tag bit flips. 

    MutatorDef & GetMutator(size_t id) {
      return mutator_lib[id];
    }

    size_t GetMutatorID(const std::string & name) {
      emp_assert(Has(mutator_name_map, name));
      return mutator_name_map[name];
    }

  public:
    SignalGPMutator(size_t _PROG_MIN_FUNC_CNT=1,
                    size_t _PROG_MAX_FUNC_CNT=8,
                    size_t _PROG_MIN_FUNC_LEN=1,
                    size_t _PROG_MAX_FUNC_LEN=32,
                    size_t _PROG_MAX_TOTAL_LEN=256,
                    int _PROG_MIN_ARG_VAL=0,
                    int _PROG_MAX_ARG_VAL=15)
      : param_lib(), param_name_map(),
        mutator_lib(), mutator_name_map(),
        prog_min_func_cnt(_PROG_MIN_FUNC_CNT),
        prog_max_func_cnt(_PROG_MAX_FUNC_CNT),
        prog_min_func_len(_PROG_MIN_FUNC_LEN),
        prog_max_func_len(_PROG_MAX_FUNC_LEN),
        prog_max_total_len(_PROG_MAX_TOTAL_LEN),
        prog_min_arg_val(_PROG_MIN_ARG_VAL),
        prog_max_arg_val(_PROG_MAX_ARG_VAL)
        {
          // Setup default rates. 
          arg_sub__per_arg__id = AddParam("ARG_SUB__PER_ARG", 0.005, "Rate to apply substitutions to instruction arguments.");
          inst_sub__per_inst__id = AddParam("INST_SUB__PER_INST", 0.005, "Per-instruction rate to apply instruction substitutions. ");
          inst_ins__per_inst__id = AddParam("INST_INS__PER_INST", 0.005, "Per-instruction rate to apply instruction insertions.");
          inst_del__per_inst__id = AddParam("INST_DEL__PER_INST", 0.005, "Per-instruction rate to apply instruction deletions.");
          slip__per_func__id = AddParam("SLIP__PER_FUNC", 0.05, "Per-function rate to apply slip-mutations.");
          func_dup__per_func__id = AddParam("FUNC_DUP__PER_FUNC", 0.05, "Per-function rate to apply function duplications.");
          func_del__per_func__id = AddParam("FUNC_DEL__PER_FUNC", 0.05, "Per-function rate to apply function deletions.");
          tag_bit_flip__per_bit__id = AddParam("TAG_BIT_FLIP__PER_BIT", 0.005, "Per-bit rate to apply tag bit flips. ");
          // Setup default mutators.
          AddMutator("FuncDup", [this](program_t & p, emp::Random & r) { return this->DefaultMutator_FuncDup(p, r); }, "Default mutator. Whole-function duplications applied at a per-function rate.");
          AddMutator("FuncDel", [this](program_t & p, emp::Random & r) { return this->DefaultMutator_FuncDel(p, r); }, "Default mutator. Whole-function deletions applied at a per-function rate.");
          AddMutator("FuncTag", [this](program_t & p, emp::Random & r) { return this->DefaultMutator_FuncTag(p, r); }, "Default mutator. Function tag mutations applied at a per-bit rate.");
          AddMutator("Slip", [this](program_t & p, emp::Random & r) { return this->DefaultMutator_Slip(p, r); }, "Default mutator. Slip mutations (multi-instruction sequence duplication/deletions) applied at a per-function rate.");
          AddMutator("Subs", [this](program_t & p, emp::Random & r) { return this->DefaultMutator_Subs(p, r); }, "Default mutator. Single-instruction substitutions applied at a per-instruction rate, argument substitutions applied at a per-argument rate, and instruction tag mutations applied at a per-bit rate.");
          AddMutator("InstInDels", [this](program_t & p, emp::Random & r) { return this->DefaultMutator_InstInDels(p, r); }, "Default mutator. Single-instruction insertions and deletions applied at a per-instruction rate.");

        }

    SignalGPMutator(const SignalGPMutator &) = default;
    SignalGPMutator(SignalGPMutator &&) = default;
    ~SignalGPMutator() { ; } 

    SignalGPMutator & operator=(const SignalGPMutator &) = default;   ///< Copy operator
    SignalGPMutator & operator=(SignalGPMutator &&) = default;        ///< Move operator

    size_t ApplyMutations(program_t & p, emp::Random & r) {
      size_t mut_cnt = 0;
      for (auto & mutator_type : mutator_lib) {
        mutator_type.last_mut_cnt = mutator_type.mutator(p, r);
        mut_cnt += mutator_type.last_mut_cnt;
      }
      return mut_cnt;
    }

    size_t ApplyMutator(const std::string & name, program_t & p, emp::Random & r) {
      const size_t id = GetMutatorID(name);
      GetMutator(id).last_mut_cnt = GetMutator(id).mutator(p, r);
      return GetMutator(id).last_mut_cnt;
    }

    size_t GetProgMinFuncCnt() const { return prog_min_func_cnt; }
    size_t GetProgMaxFuncCnt() const { return prog_max_func_cnt; }
    size_t GetProgMinFuncLen() const { return prog_min_func_len; }
    size_t GetProgMaxFuncLen() const { return prog_max_func_len; }
    size_t GetProgMaxTotalLen() const { return prog_max_total_len; }
    int GetProgMinArgVal() const { return prog_min_arg_val; }
    int GetProgMaxArgVal() const { return prog_max_arg_val; }
    void SetProgMinFuncCnt(size_t val) { prog_min_func_cnt = val; }
    void SetProgMaxFuncCnt(size_t val) { prog_max_func_cnt = val; }
    void SetProgMinFuncLen(size_t val) { prog_min_func_len = val; }
    void SetProgMaxFuncLen(size_t val) { prog_max_func_len = val; }
    void SetProgMaxTotalLen(size_t val) { prog_max_total_len = val; }
    void SetProgMinArgVal(int val) { prog_min_arg_val = val; }
    void SetProgMaxArgVal(int val) { prog_max_arg_val = val; }

    // Getters and setters for default mutation rates. 
    double ARG_SUB__PER_ARG() const { return GetParam(arg_sub__per_arg__id); }
    double INST_SUB__PER_INST() const { return GetParam(inst_sub__per_inst__id); }
    double INST_INS__PER_INST() const { return GetParam(inst_ins__per_inst__id); }
    double INST_DEL__PER_INST() const { return GetParam(inst_del__per_inst__id); }
    double SLIP__PER_FUNC() const { return GetParam(slip__per_func__id); }
    double FUNC_DUP__PER_FUNC() const { return GetParam(func_dup__per_func__id); }
    double FUNC_DEL__PER_FUNC() const { return GetParam(func_del__per_func__id); }
    double TAG_BIT_FLIP__PER_BIT() const { return GetParam(tag_bit_flip__per_bit__id); }
    void ARG_SUB__PER_ARG(double val) { SetParam(arg_sub__per_arg__id, val); }
    void INST_SUB__PER_INST(double val) { SetParam(inst_sub__per_inst__id, val); }
    void INST_INS__PER_INST(double val) { SetParam(inst_ins__per_inst__id, val); }
    void INST_DEL__PER_INST(double val) { SetParam(inst_del__per_inst__id, val); }
    void SLIP__PER_FUNC(double val) { SetParam(slip__per_func__id, val); }
    void FUNC_DUP__PER_FUNC(double val) { SetParam(func_dup__per_func__id, val); }
    void FUNC_DEL__PER_FUNC(double val) { SetParam(func_del__per_func__id, val); }
    void TAG_BIT_FLIP__PER_BIT(double val) { SetParam(tag_bit_flip__per_bit__id, val); }

    /// Return the ID of the mutation rate type that has the specified name.
    /// Return (size_t)-1 if nothing found. 
    size_t GetParamID(const std::string & name) const { 
      emp_assert(Has(param_name_map, name), name);
      return Find(param_name_map, name, (size_t)-1);
    }

    /// Return the rate associated with the specified mutation rate ID. 
    double GetParam(size_t id) const { return param_lib[id].param; }

    /// Return the rate associated with the specified mutation rate name.
    double GetParam(const std::string & param_name) const { return GetParam(GetParamID(param_name)); }

    const std::string & GetDesc(size_t id) const { return param_lib[id].desc; }

    const std::string & GetDesc(const std::string & name) { return GetDesc(GetParamID(name)); }

    const std::string & GetName(size_t id) const { return param_lib[id].name; } 
    
    /// Add a known mutation rate to mutator. 
    /// Return ID of that rate.
    size_t AddParam(const std::string & name, double param, const std::string & desc="") {
      emp_assert(!Has(param_name_map, name));
      const size_t id = param_lib.size();
      param_lib.emplace_back(name, param, desc);
      param_name_map[name] = id;
      return id;
    }

    /// Modify existing mutation rate (by name).  
    void SetParam(const std::string & name, double param) {
      emp_assert(Has(param_name_map, name));
      SetParam(GetParamID(name), param);
    }

    /// Modify existing mutation rate (by ID).
    void SetParam(size_t id, double param) {
      emp_assert(id < param_lib.size());
      param_lib[id].param = param;
    }

    // -------------------------------------------------
    // ---- Functions related to mutation operators ----
    void AddMutator(const std::string & name, const mutator_fun_t & mut_fun, const std::string & desc="") {
      emp_assert(!Has(mutator_name_map, name));
      mutator_name_map[name] = mutator_lib.size();
      mutator_lib.emplace_back(name, mut_fun, desc);
    } 

    void RemoveMutator(const std::string & name) {
      emp_assert(Has(name, mutator_name_map));
      emp_assert(mutator_lib.size() > 0);
      size_t rm_id = mutator_name_map[name];
      if (rm_id < mutator_lib.size()-1) {
        size_t mv_id = mutator_lib.size()-1;
        mutator_name_map[mutator_lib[mv_id].name] = rm_id;
        std::swap(mutator_lib[rm_id], mutator_lib[mv_id]);
        mutator_name_map.erase(name);
      } 
      mutator_lib.pop_back();
    }

    void ClearMutators() {
      mutator_lib.clear();
      mutator_name_map.clear();
    }

    // ---------------------------------------
    // --- Default mutator implementations ---
    size_t DefaultMutator_FuncDup(program_t & program, emp::Random & rnd) {
      size_t mut_cnt = 0;
      size_t expected_prog_len = program.GetInstCnt();
      // Perform function duplications!
      size_t orig_func_wall = program.GetSize();
      for (size_t fID = 0; fID < orig_func_wall; ++fID) {
        // Should we duplicate this function?
        if (fID < orig_func_wall &&
            rnd.P(FUNC_DUP__PER_FUNC()) && 
            program.GetSize() < GetProgMaxFuncCnt() && 
            expected_prog_len + program[fID].GetSize() <= GetProgMaxTotalLen()) {
            // Duplicate!
            program.PushFunction(program[fID]);
            expected_prog_len += program[fID].GetSize();
            ++mut_cnt;
        }
      }
      return mut_cnt;
    }

    size_t DefaultMutator_FuncDel(program_t & program, emp::Random & rnd) {
      size_t mut_cnt = 0;
      size_t expected_prog_len = program.GetInstCnt();
      // Perform function deletions!
      for (int fID = 0; fID < program.GetSize(); ++fID) {
        // Should we delete this function?
        if (rnd.P(FUNC_DEL__PER_FUNC()) &&
            program.GetSize() > GetProgMinFuncCnt()) 
          {
            expected_prog_len -= program[(size_t)fID].GetSize();
            program[(size_t)fID] = program[program.GetSize() - 1];
            program.program.resize(program.GetSize() - 1);
            ++mut_cnt;
            fID -= 1;
            continue;
        }
      }
      return mut_cnt;
    }

    size_t DefaultMutator_FuncTag(program_t & program, emp::Random & rnd) {
      size_t mut_cnt = 0;
      // Perform function tag mutations!
      for (size_t fID = 0; fID < program.GetSize(); ++fID) {
        tag_t & tag = program[fID].GetAffinity();
        for (size_t i = 0; i < tag.GetSize(); ++i) {
          if (rnd.P(TAG_BIT_FLIP__PER_BIT())) {
            tag.Toggle(i);
            ++mut_cnt;
          }
        }
      }
      return mut_cnt;
    }

    size_t DefaultMutator_Slip(program_t & program, emp::Random & rnd) {
      size_t mut_cnt = 0;
      size_t expected_prog_len = program.GetInstCnt();
      // Perform per-function slip-mutations!
      for (size_t fID = 0; fID < program.GetSize(); ++fID) {
        if (rnd.P(SLIP__PER_FUNC())) {
          size_t begin = rnd.GetUInt(program[fID].GetSize());
          size_t end = rnd.GetUInt(program[fID].GetSize());
          const bool dup = begin < end;
          const bool del = begin > end;
          const int dup_size = (int)end - (int)begin;
          const int del_size = (int)begin - (int)end;
          if (dup && 
              (expected_prog_len + (size_t)dup_size <= GetProgMaxTotalLen()) &&
              (program[fID].GetSize() + (size_t)dup_size <= GetProgMaxFuncLen())) {
            // Duplicate begin:end!
            const size_t new_size = program[fID].GetSize() + (size_t)dup_size;
            fun_t new_fun(program[fID].GetAffinity());
            for (size_t i = 0; i < new_size; ++i) {
              if (i < end) new_fun.PushInst(program[fID][i]);
              else new_fun.PushInst(program[fID][i - (size_t)dup_size]);
            }
            program[fID] = new_fun;
            ++mut_cnt;
            expected_prog_len += (size_t)dup_size;
          } else if (del && 
                     (program[fID].GetSize() - (size_t)del_size) >= GetProgMinFuncLen()) {
            // Delete end:begin!
            fun_t new_fun(program[fID].GetAffinity());
            for (size_t i = 0; i < end; ++i)
              new_fun.PushInst(program[fID][i]);
            for (size_t i = begin; i < program[fID].GetSize(); ++i)
              new_fun.PushInst(program[fID][i]);
            program[fID] = new_fun;
            ++mut_cnt;
            expected_prog_len -= (size_t)del_size;
          }
        }
      }
      return mut_cnt;
    }

    size_t DefaultMutator_Subs(program_t & program, emp::Random & rnd) {
      size_t mut_cnt = 0;
      // Perform single-instruction substitutions!
      for (size_t fID = 0; fID < program.GetSize(); ++fID) {
        for (size_t iID = 0; iID < program[fID].GetSize(); ++iID) {
          inst_t & inst = program[fID][iID];
          
          // Mutate affinity
          tag_t & tag = inst.affinity;
          for (size_t k = 0; k < tag.GetSize(); ++k) {
            if (rnd.P(TAG_BIT_FLIP__PER_BIT())) {
              tag.Toggle(k);
              ++mut_cnt;
            }
          }

          // Mutate instruction operation.
          if (rnd.P(INST_SUB__PER_INST())) {
            inst.id = rnd.GetUInt(program.GetInstLib()->GetSize());
            ++mut_cnt;
          }

          // Mutate instruction arguments.
          for (size_t k = 0; k < hardware_t::MAX_INST_ARGS; ++k) {
            if (rnd.P(ARG_SUB__PER_ARG())) {
              inst.args[k] = rnd.GetInt(GetProgMinArgVal(), GetProgMaxArgVal());
              ++mut_cnt;
            }
          }
        } 
      }
      return mut_cnt;
    }

    size_t DefaultMutator_InstInDels(program_t & program, emp::Random & rnd) {
      size_t mut_cnt = 0;
      size_t expected_prog_len = program.GetInstCnt();
            // Perform single-instruction insertion/deletions
      for (size_t fID = 0; fID < program.GetSize(); ++fID) {
        fun_t new_fun(program[fID].GetAffinity());
        size_t expected_func_len = program[fID].GetSize();
        // Compute number and location of insertions.
        const uint32_t num_ins = rnd.GetRandBinomial(program[fID].GetSize(), INST_INS__PER_INST());
        emp::vector<size_t> ins_locs;
        if (num_ins > 0) {
          ins_locs = emp::RandomUIntVector(rnd, num_ins, 0, program[fID].GetSize());
          std::sort(ins_locs.begin(), ins_locs.end(), std::greater<size_t>());
        }
        size_t rhead = 0;
        while (rhead < program[fID].GetSize()) {
          // Should we insert?
          if (ins_locs.size() > 0) {
            if (rhead >= ins_locs.back() && 
                expected_func_len < GetProgMaxFuncLen() && 
                expected_prog_len < GetProgMaxTotalLen()) {
              // Insert a random instruction.
              new_fun.PushInst(GenRandSignalGPInst(rnd, *(program.GetInstLib()), GetProgMinArgVal(), GetProgMaxArgVal()));
              ++mut_cnt;
              ++expected_prog_len;
              ++expected_func_len;
              ins_locs.pop_back();
              continue;
            }
          }

          // Should we delete this instruction?
          if (rnd.P(INST_DEL__PER_INST()) && 
              expected_func_len > GetProgMinFuncLen()) {
            // Delete this instruction!
            ++mut_cnt;
            --expected_func_len;
            --expected_prog_len;
          } else {
            new_fun.PushInst(program[fID][rhead]);
          }
          ++rhead;
        }
        program[fID] = new_fun;
      }
      return mut_cnt;
    }

    void Print(std::ostream & os=std::cout) {
      os << "== MUTATOR PARAMETERS ==\n";
      os << "PROG_MIN_FUNC_CNT = " << GetProgMinFuncCnt() << "\n";
      os << "PROG_MAX_FUNC_CNT = " << GetProgMaxFuncCnt() << "\n";
      os << "PROG_MIN_FUNC_LEN = " << GetProgMinFuncLen() << "\n";
      os << "PROG_MAX_FUNC_LEN = " << GetProgMaxFuncLen() << "\n";
      os << "PROG_MAX_TOTAL_LEN = " << GetProgMaxTotalLen() << "\n";
      os << "PROG_MIN_ARG_VAL = " << GetProgMinArgVal() << "\n";
      os << "PROG_MAX_ARG_VAL = " << GetProgMaxArgVal() << "\n";
      for (auto & param : param_lib) {
        os << param.name << " = " << param.param << " (" << param.desc << ")\n";
      }
      os << "== MUTATORS ==\n";
      for (auto & mutator_type : mutator_lib) {
        os << mutator_type.name << " : " << mutator_type.desc << "\n";
      } 
    }
  };


}

#endif