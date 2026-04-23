// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__PeakInfo
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "ROOT/RConfig.hxx"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "ctools.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *PeakInfo_Dictionary();
   static void PeakInfo_TClassManip(TClass*);
   static void *new_PeakInfo(void *p = nullptr);
   static void *newArray_PeakInfo(Long_t size, void *p);
   static void delete_PeakInfo(void *p);
   static void deleteArray_PeakInfo(void *p);
   static void destruct_PeakInfo(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::PeakInfo*)
   {
      ::PeakInfo *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::PeakInfo));
      static ::ROOT::TGenericClassInfo 
         instance("PeakInfo", "ctools.h", 17,
                  typeid(::PeakInfo), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &PeakInfo_Dictionary, isa_proxy, 4,
                  sizeof(::PeakInfo) );
      instance.SetNew(&new_PeakInfo);
      instance.SetNewArray(&newArray_PeakInfo);
      instance.SetDelete(&delete_PeakInfo);
      instance.SetDeleteArray(&deleteArray_PeakInfo);
      instance.SetDestructor(&destruct_PeakInfo);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::PeakInfo*)
   {
      return GenerateInitInstanceLocal(static_cast<::PeakInfo*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::PeakInfo*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *PeakInfo_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::PeakInfo*>(nullptr))->GetClass();
      PeakInfo_TClassManip(theClass);
   return theClass;
   }

   static void PeakInfo_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_PeakInfo(void *p) {
      return  p ? new(p) ::PeakInfo : new ::PeakInfo;
   }
   static void *newArray_PeakInfo(Long_t nElements, void *p) {
      return p ? new(p) ::PeakInfo[nElements] : new ::PeakInfo[nElements];
   }
   // Wrapper around operator delete
   static void delete_PeakInfo(void *p) {
      delete (static_cast<::PeakInfo*>(p));
   }
   static void deleteArray_PeakInfo(void *p) {
      delete [] (static_cast<::PeakInfo*>(p));
   }
   static void destruct_PeakInfo(void *p) {
      typedef ::PeakInfo current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::PeakInfo

namespace ROOT {
   static TClass *vectorlEPeakInfogR_Dictionary();
   static void vectorlEPeakInfogR_TClassManip(TClass*);
   static void *new_vectorlEPeakInfogR(void *p = nullptr);
   static void *newArray_vectorlEPeakInfogR(Long_t size, void *p);
   static void delete_vectorlEPeakInfogR(void *p);
   static void deleteArray_vectorlEPeakInfogR(void *p);
   static void destruct_vectorlEPeakInfogR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<PeakInfo>*)
   {
      vector<PeakInfo> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<PeakInfo>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<PeakInfo>", -2, "vector", 428,
                  typeid(vector<PeakInfo>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEPeakInfogR_Dictionary, isa_proxy, 4,
                  sizeof(vector<PeakInfo>) );
      instance.SetNew(&new_vectorlEPeakInfogR);
      instance.SetNewArray(&newArray_vectorlEPeakInfogR);
      instance.SetDelete(&delete_vectorlEPeakInfogR);
      instance.SetDeleteArray(&deleteArray_vectorlEPeakInfogR);
      instance.SetDestructor(&destruct_vectorlEPeakInfogR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<PeakInfo> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<PeakInfo>","std::vector<PeakInfo, std::allocator<PeakInfo> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<PeakInfo>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEPeakInfogR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<PeakInfo>*>(nullptr))->GetClass();
      vectorlEPeakInfogR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEPeakInfogR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEPeakInfogR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<PeakInfo> : new vector<PeakInfo>;
   }
   static void *newArray_vectorlEPeakInfogR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<PeakInfo>[nElements] : new vector<PeakInfo>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEPeakInfogR(void *p) {
      delete (static_cast<vector<PeakInfo>*>(p));
   }
   static void deleteArray_vectorlEPeakInfogR(void *p) {
      delete [] (static_cast<vector<PeakInfo>*>(p));
   }
   static void destruct_vectorlEPeakInfogR(void *p) {
      typedef vector<PeakInfo> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<PeakInfo>

namespace {
  void TriggerDictionaryInitialization_libPeakInfo_Impl() {
    static const char* headers[] = {
"ctools.h",
nullptr
    };
    static const char* includePaths[] = {
"/opt/gentoo/usr/include/root",
"/home/wangyp/1ton/ReConstruction/TimeCali/include",
"/home/wangyp/JinpingPackage/JSAP-install/Analysis",
"/home/wangyp/JinpingPackage/JSAP-install/Simulation/DataType/include",
"/home/wangyp/JinpingPackage/JSAP-install/Reconstruction/src",
"/home/wangyp/JinpingPackage/JSAP-install",
"/home/wangyp/1ton/ReConstruction/TimeCali",
"/opt/gentoo/usr/include/root",
"/home/wangyp/1ton/ReConstruction/TimeCali/buildtemp/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libPeakInfo dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$ctools.h")))  PeakInfo;
namespace std{template <typename _Tp> class __attribute__((annotate("$clingAutoload$bits/allocator.h")))  __attribute__((annotate("$clingAutoload$string")))  allocator;
}
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libPeakInfo dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "ctools.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"PeakInfo", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libPeakInfo",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libPeakInfo_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libPeakInfo_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libPeakInfo() {
  TriggerDictionaryInitialization_libPeakInfo_Impl();
}
