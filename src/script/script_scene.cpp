#include <lunar/script/script_api.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>
#include <jni.h>

extern "C"
{
    /*
     * Class:     dev_lunar_core_Scene
     * Method:    _getActiveScene
     * Signature: ()Ldev/lunar/core/internal/NativeObjectHandle;
     */
    JNIEXPORT jobject JNICALL Java_dev_lunar_core_Scene__1getActiveScene
    (JNIEnv* env, jclass)
    {
        auto& vm = Script::getVmFromEnv(env);
        auto scene_id = Core::getActiveScene()
                            .getId();

        return vm.createNativeHandle(scene_id);
    }

    /*
     * Class:     dev_lunar_core_Scene
     * Method:    getName
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_dev_lunar_core_Scene_getName
    (JNIEnv* env, jobject object)
    {
        auto& vm = Script::getVmFromEnv(env);
        auto scene_id = vm.getNativeHandle(object);
        return Script::createManagedString(
            Core::getSceneById(scene_id)
                .getName()
        );
    }

    /*
     * Class:     dev_lunar_core_Scene
     * Method:    _getGameObject
     * Signature: (Ljava/lang/String;)Ldev/lunar/core/internal/NativeObjectHandle;
     */
    JNIEXPORT jobject JNICALL Java_dev_lunar_core_Scene__1getGameObject
    (JNIEnv* env, jobject scene_object, jstring name)
    {
        auto& vm = Script::getVmFromEnv(env);
        auto scene_id = vm.getNativeHandle(scene_object);
        const char* object_name = env->GetStringUTFChars(name, 0);

        auto& scene = Core::getSceneById(scene_id);
        auto& object = scene.getGameObject(object_name);
        // TODO: return null if not found
        return vm.createNativeHandle(object.getId());
    }

    /*
     * Class:     dev_lunar_core_Scene
     * Method:    _getGameObjects
     * Signature: ()Ljava/util/List;
     */
    JNIEXPORT jobject JNICALL Java_dev_lunar_core_Scene__1getGameObjects
    (JNIEnv* env, jobject object)
    {
        auto& vm = Script::getVmFromEnv(env);
        
        auto scene_id = vm.getNativeHandle(object);
        auto& scene = Core::getSceneById(scene_id);
        auto& scene_objects = scene.getGameObjects();
        auto array_list = Script::JavaArrayList(scene_objects.size());
        for (auto& game_object : scene_objects) {
            array_list.add(vm.createNativeHandle(game_object.getId()));
        }
        return array_list.object;
    }
}
