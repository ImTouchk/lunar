#include <lunar/script/script_api.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/script/api/math.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>

extern "C"
{
    ///*
    // * Class:     dev_lunar_core_GameObject
    // * Method:    getName
    // * Signature: ()Ljava/lang/String;
    // */
    //JNIEXPORT jstring JNICALL Java_dev_lunar_core_GameObject_getName
    //(JNIEnv* env, jobject object)
    //{
    //    auto& vm = Script::getVmFromEnv(env);

    //    auto object_id = vm.getNativeHandle(object);
    //    auto& game_object = Core::getActiveScene()
    //                            .getGameObject(object_id);

    //    return Script::createManagedString(game_object.getName());
    //}

    ///*
    // * Class:     dev_lunar_core_GameObject
    // * Method:    _getParent
    // * Signature: ()Ldev/lunar/core/internal/NativeObjectHandle;
    // */
    //JNIEXPORT jobject JNICALL Java_dev_lunar_core_GameObject__1getParent
    //(JNIEnv* env, jobject object)
    //{
    //    auto& vm = Script::getVmFromEnv(env);

    //    auto object_id = vm.getNativeHandle(object);
    //    auto& game_object = Core::getActiveScene()
    //                            .getGameObject(object_id);

    //    auto* parent = game_object.getParent();
    //    if (parent != nullptr)
    //        return vm.createNativeHandle(parent->getId());
    //    else
    //        return nullptr;
    //}
}
