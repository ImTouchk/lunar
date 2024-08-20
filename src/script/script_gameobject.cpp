#include <lunar/script/script_api.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/scene.hpp>

namespace Script
{
    inline Core::GameObject& getGameObjectFromHandle(jobject& object)
    {
        return Core::getActiveScene()
                .getGameObject(
                    getMainVm()
                        .getWrapper(VM_GAMEOBJ_WRAPPER)
                        .getHandle(object)
                );
    }
}

extern "C"
{
    /*
    * Class:     dev_lunar_core_GameObject
    * Method:    getName
    * Signature: ()Ljava/lang/String;
    */
    JNIEXPORT jstring JNICALL Java_dev_lunar_core_GameObject_getName
    (JNIEnv* env, jobject object)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        return Script::createManagedString(game_object.getName());
    }
}
