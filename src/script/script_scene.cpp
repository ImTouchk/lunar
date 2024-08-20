#include <lunar/script/script_api.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>
#include <jni.h>

namespace Script
{
    inline Core::Scene& getSceneFromVmHandle(jobject& object)
    {
        return Core::getScene(
                    getMainVm()
                        .getWrapper(VM_SCENE_WRAPPER)
                        .getHandle(object)
                );
    }
}

extern "C"
{
    /*
    * Class:     dev_lunar_core_Scene
    * Method:    getActiveScene
    * Signature: ()Ldev/lunar/core/Scene;
    */
    JNIEXPORT jobject JNICALL Java_dev_lunar_core_Scene_getActiveScene
    (JNIEnv* env, jclass klass)
    {
        return Script::getMainVm().getWrapper(Script::VM_SCENE_WRAPPER)
            .createHandle(Core::getActiveScene().getNameHash());
    }

    /*
     * Class:     dev_lunar_core_Scene
     * Method:    getName
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_dev_lunar_core_Scene_getName
    (JNIEnv* env, jobject object)
    {
        auto& scene = Script::getSceneFromVmHandle(object);
        return Script::createManagedString(scene.getName());
    }

    /*
     * Class:     dev_lunar_core_Scene
     * Method:    getGameObjects
     * Signature: ()Ljava/util/List;
     */
    JNIEXPORT jobject JNICALL Java_dev_lunar_core_Scene_getGameObjects
    (JNIEnv* env, jobject object)
    {
        auto& scene = Script::getSceneFromVmHandle(object);
        auto& game_objects = scene.getGameObjects();
        auto array_list = Script::JavaArrayList(game_objects.size());
        for(auto& game_object : game_objects)
        {
            array_list.add(
                Script::getMainVm()
                    .getWrapper(Script::VM_GAMEOBJ_WRAPPER)
                    .createHandle(game_object.getId())
            );
        }

        return array_list.object;
    }
}
