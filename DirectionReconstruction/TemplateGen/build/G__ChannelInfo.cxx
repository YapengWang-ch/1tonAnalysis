// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__ChannelInfo
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
#include "/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen/include/ChannelInfo.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static TClass *ChannelInfo_t_Dictionary();
   static void ChannelInfo_t_TClassManip(TClass*);
   static void *new_ChannelInfo_t(void *p = nullptr);
   static void *newArray_ChannelInfo_t(Long_t size, void *p);
   static void delete_ChannelInfo_t(void *p);
   static void deleteArray_ChannelInfo_t(void *p);
   static void destruct_ChannelInfo_t(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::ChannelInfo_t*)
   {
      ::ChannelInfo_t *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::ChannelInfo_t));
      static ::ROOT::TGenericClassInfo 
         instance("ChannelInfo_t", "include/ChannelInfo.h", 7,
                  typeid(::ChannelInfo_t), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &ChannelInfo_t_Dictionary, isa_proxy, 4,
                  sizeof(::ChannelInfo_t) );
      instance.SetNew(&new_ChannelInfo_t);
      instance.SetNewArray(&newArray_ChannelInfo_t);
      instance.SetDelete(&delete_ChannelInfo_t);
      instance.SetDeleteArray(&deleteArray_ChannelInfo_t);
      instance.SetDestructor(&destruct_ChannelInfo_t);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::ChannelInfo_t*)
   {
      return GenerateInitInstanceLocal(static_cast<::ChannelInfo_t*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::ChannelInfo_t*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *ChannelInfo_t_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const ::ChannelInfo_t*>(nullptr))->GetClass();
      ChannelInfo_t_TClassManip(theClass);
   return theClass;
   }

   static void ChannelInfo_t_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_ChannelInfo_t(void *p) {
      return  p ? new(p) ::ChannelInfo_t : new ::ChannelInfo_t;
   }
   static void *newArray_ChannelInfo_t(Long_t nElements, void *p) {
      return p ? new(p) ::ChannelInfo_t[nElements] : new ::ChannelInfo_t[nElements];
   }
   // Wrapper around operator delete
   static void delete_ChannelInfo_t(void *p) {
      delete (static_cast<::ChannelInfo_t*>(p));
   }
   static void deleteArray_ChannelInfo_t(void *p) {
      delete [] (static_cast<::ChannelInfo_t*>(p));
   }
   static void destruct_ChannelInfo_t(void *p) {
      typedef ::ChannelInfo_t current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::ChannelInfo_t

namespace ROOT {
   static TClass *vectorlEintgR_Dictionary();
   static void vectorlEintgR_TClassManip(TClass*);
   static void *new_vectorlEintgR(void *p = nullptr);
   static void *newArray_vectorlEintgR(Long_t size, void *p);
   static void delete_vectorlEintgR(void *p);
   static void deleteArray_vectorlEintgR(void *p);
   static void destruct_vectorlEintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<int>*)
   {
      vector<int> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<int>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<int>", -2, "vector", 428,
                  typeid(vector<int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEintgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<int>) );
      instance.SetNew(&new_vectorlEintgR);
      instance.SetNewArray(&newArray_vectorlEintgR);
      instance.SetDelete(&delete_vectorlEintgR);
      instance.SetDeleteArray(&deleteArray_vectorlEintgR);
      instance.SetDestructor(&destruct_vectorlEintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<int> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<int>","std::vector<int, std::allocator<int> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<int>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<int>*>(nullptr))->GetClass();
      vectorlEintgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEintgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<int> : new vector<int>;
   }
   static void *newArray_vectorlEintgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<int>[nElements] : new vector<int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEintgR(void *p) {
      delete (static_cast<vector<int>*>(p));
   }
   static void deleteArray_vectorlEintgR(void *p) {
      delete [] (static_cast<vector<int>*>(p));
   }
   static void destruct_vectorlEintgR(void *p) {
      typedef vector<int> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<int>

namespace ROOT {
   static TClass *vectorlEdoublegR_Dictionary();
   static void vectorlEdoublegR_TClassManip(TClass*);
   static void *new_vectorlEdoublegR(void *p = nullptr);
   static void *newArray_vectorlEdoublegR(Long_t size, void *p);
   static void delete_vectorlEdoublegR(void *p);
   static void deleteArray_vectorlEdoublegR(void *p);
   static void destruct_vectorlEdoublegR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<double>*)
   {
      vector<double> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<double>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<double>", -2, "vector", 428,
                  typeid(vector<double>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEdoublegR_Dictionary, isa_proxy, 0,
                  sizeof(vector<double>) );
      instance.SetNew(&new_vectorlEdoublegR);
      instance.SetNewArray(&newArray_vectorlEdoublegR);
      instance.SetDelete(&delete_vectorlEdoublegR);
      instance.SetDeleteArray(&deleteArray_vectorlEdoublegR);
      instance.SetDestructor(&destruct_vectorlEdoublegR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<double> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<double>","std::vector<double, std::allocator<double> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<double>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEdoublegR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<double>*>(nullptr))->GetClass();
      vectorlEdoublegR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEdoublegR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEdoublegR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<double> : new vector<double>;
   }
   static void *newArray_vectorlEdoublegR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<double>[nElements] : new vector<double>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEdoublegR(void *p) {
      delete (static_cast<vector<double>*>(p));
   }
   static void deleteArray_vectorlEdoublegR(void *p) {
      delete [] (static_cast<vector<double>*>(p));
   }
   static void destruct_vectorlEdoublegR(void *p) {
      typedef vector<double> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<double>

namespace ROOT {
   static TClass *vectorlEChannelInfo_tgR_Dictionary();
   static void vectorlEChannelInfo_tgR_TClassManip(TClass*);
   static void *new_vectorlEChannelInfo_tgR(void *p = nullptr);
   static void *newArray_vectorlEChannelInfo_tgR(Long_t size, void *p);
   static void delete_vectorlEChannelInfo_tgR(void *p);
   static void deleteArray_vectorlEChannelInfo_tgR(void *p);
   static void destruct_vectorlEChannelInfo_tgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<ChannelInfo_t>*)
   {
      vector<ChannelInfo_t> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<ChannelInfo_t>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<ChannelInfo_t>", -2, "vector", 428,
                  typeid(vector<ChannelInfo_t>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEChannelInfo_tgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<ChannelInfo_t>) );
      instance.SetNew(&new_vectorlEChannelInfo_tgR);
      instance.SetNewArray(&newArray_vectorlEChannelInfo_tgR);
      instance.SetDelete(&delete_vectorlEChannelInfo_tgR);
      instance.SetDeleteArray(&deleteArray_vectorlEChannelInfo_tgR);
      instance.SetDestructor(&destruct_vectorlEChannelInfo_tgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<ChannelInfo_t> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<ChannelInfo_t>","std::vector<ChannelInfo_t, std::allocator<ChannelInfo_t> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<ChannelInfo_t>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEChannelInfo_tgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<ChannelInfo_t>*>(nullptr))->GetClass();
      vectorlEChannelInfo_tgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEChannelInfo_tgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEChannelInfo_tgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<ChannelInfo_t> : new vector<ChannelInfo_t>;
   }
   static void *newArray_vectorlEChannelInfo_tgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<ChannelInfo_t>[nElements] : new vector<ChannelInfo_t>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEChannelInfo_tgR(void *p) {
      delete (static_cast<vector<ChannelInfo_t>*>(p));
   }
   static void deleteArray_vectorlEChannelInfo_tgR(void *p) {
      delete [] (static_cast<vector<ChannelInfo_t>*>(p));
   }
   static void destruct_vectorlEChannelInfo_tgR(void *p) {
      typedef vector<ChannelInfo_t> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<ChannelInfo_t>

namespace ROOT {
   // Registration Schema evolution read functions
   int RecordReadRules_libChannelInfo() {
      return 0;
   }
   static int _R__UNIQUE_DICT_(ReadRules_libChannelInfo) = RecordReadRules_libChannelInfo();R__UseDummy(_R__UNIQUE_DICT_(ReadRules_libChannelInfo));
} // namespace ROOT
namespace {
  void TriggerDictionaryInitialization_libChannelInfo_Impl() {
    static const char* headers[] = {
"/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen/include/ChannelInfo.h",
nullptr
    };
    static const char* includePaths[] = {
"/opt/gentoo/usr/include/root",
"/home/wangyp/JinpingPackage/JSAP-install/Analysis",
"/home/wangyp/JinpingPackage/JSAP-install/Simulation/DataType/include",
"/home/wangyp/JinpingPackage/JSAP-install/Reconstruction/src",
"/home/wangyp/JinpingPackage/JSAP-install",
"/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen/include",
"/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen",
"/opt/gentoo/usr/include/root",
"/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen/build/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libChannelInfo dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen/include/ChannelInfo.h")))  ChannelInfo_t;
namespace std{template <typename _Tp> class __attribute__((annotate("$clingAutoload$bits/allocator.h")))  __attribute__((annotate("$clingAutoload$string")))  allocator;
}
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libChannelInfo dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "/home/wangyp/1ton/WaterPhaseII/DirectionReconstruction/TemplateGen/include/ChannelInfo.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"ChannelInfo_t", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libChannelInfo",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libChannelInfo_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libChannelInfo_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libChannelInfo() {
  TriggerDictionaryInitialization_libChannelInfo_Impl();
}
