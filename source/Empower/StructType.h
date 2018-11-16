/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  StructType.h
 *  @brief StructType maps variables to a MemoryImage; Struct is an instance of StructType
 */

#ifndef EMP_EMPOWER_STRUCT_TYPE_H
#define EMP_EMPOWER_STRUCT_TYPE_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"

#include "MemoryImage.h"
#include "TypeManager.h"
#include "VarInfo.h"

namespace emp {

  class StructType {
  private:
    emp::vector<VarInfo> vars;   ///< Set of member variables declared in this structure.
    TypeManager & type_manager;  ///< TypeManager to track type information in this structure.
    size_t num_bytes;            ///< How big are structs of this type?
    bool active;                 ///< Have Structs of this type been built?  If so, do not extend.

  public:
    StructType(TypeManager & _tmanager) : type_manager(_tmanager), num_bytes(0), active(false) { ; }
    ~StructType() { ; }

    size_t GetSize() const { return num_bytes; }  ///< How many bytes in Structs of this type?
    bool IsActive() const { return active; }      ///< Have any Structs of this type been built?

    /// And a new member variable to structs of this type.
    template <typename T>
    void AddMemberVar(const std::string & name) {
      emp_assert(active == false, "Cannot add member variables to an instantiated struct!");
      const Type & type = type_manager.GetType<T>();
      vars.push_back(type, name, num_bytes);
      num_bytes += type.GetSize();
    }

    /// Construct a memory image using all default constructors.
    void DefaultConstruct(MemoryImage & memory) const {
      memory.resize(num_bytes);
      for (auto & x : vars) x.DefaultConstruct(memory);
      active = true;
    }
  };

  /// An instance of a struct type.
  class Struct {
  private:
    const StructType & type;  ///< What type is this Struct (i.e., what members does it have?)
    MemoryImage memory;       ///< Raw memory for storing struct information.
  public:
    Struct(const StructType & _type) : type(_type), memory(type.GetSize()) {
      type.Initialize(memory);
    }
  };
}

#endif