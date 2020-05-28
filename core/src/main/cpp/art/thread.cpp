//
// Created by canyie on 2020/3/31.
//

#include "thread.h"

using namespace pine::art;

Thread* (*Thread::current)() = nullptr;

pthread_key_t* Thread::key_self = nullptr;

jobject (*Thread::new_local_ref)(JNIEnv*, void*) = nullptr;

jweak (*Thread::add_weak_global_ref)(JavaVM*, Thread*, void*) = nullptr;

void* (*Thread::decode_jobject)(Thread*, jobject) = nullptr;

void Thread::Init(const ElfImg* handle) {
    if (Android::version < Android::VERSION_N) {
        current = reinterpret_cast<Thread* (*)()>(handle->GetSymbolAddress(
                "_ZN3art6Thread7CurrentEv")); // art::Thread::Current()
        if (UNLIKELY(!current)) {
            key_self = static_cast<pthread_key_t*>(handle->GetSymbolAddress(
                    "_ZN3art6Thread17pthread_key_self_E")); // art::Thread::pthread_key_self_
        }
    }

    new_local_ref = reinterpret_cast<jobject (*)(JNIEnv*, void*)>(handle->GetSymbolAddress(
            "_ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE")); // art::JNIEnvExt::NewLocalRef(art::mirror::Object *)

    if (UNLIKELY(!new_local_ref)) {
        LOGW("JNIEnvExt::NewLocalRef is unavailable, try JavaVMExt::AddWeakGlobalReference");
        const char* add_global_weak_ref_symbol;
        if (Android::version < Android::VERSION_M) {
            // art::JavaVMExt::AddWeakGlobalReference(art::Thread *, art::mirror::Object *)
            add_global_weak_ref_symbol = "_ZN3art9JavaVMExt22AddWeakGlobalReferenceEPNS_6ThreadEPNS_6mirror6ObjectE";
        } else if (Android::version < Android::VERSION_O) {
            // art::JavaVMExt::AddWeakGlobalRef(art::Thread *, art::mirror::Object *)
            add_global_weak_ref_symbol = "_ZN3art9JavaVMExt16AddWeakGlobalRefEPNS_6ThreadEPNS_6mirror6ObjectE";
        } else {
            // art::JavaVMExt::AddWeakGlobalRef(art::Thread *, art::ObjPtr<art::mirror::Object>)
            add_global_weak_ref_symbol = "_ZN3art9JavaVMExt16AddWeakGlobalRefEPNS_6ThreadENS_6ObjPtrINS_6mirror6ObjectEEE";
        }
        add_weak_global_ref = reinterpret_cast<jweak (*)(JavaVM*, Thread*, void*)>(
                handle->GetSymbolAddress(add_global_weak_ref_symbol));
    }

    decode_jobject = reinterpret_cast<void* (*)(Thread*, jobject)>(handle->GetSymbolAddress(
            "_ZNK3art6Thread13DecodeJObjectEP8_jobject")); // art::Thread::DecodeJObject(_jobject *)
}
