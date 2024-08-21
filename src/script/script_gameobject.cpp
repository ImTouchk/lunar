#include <lunar/script/script_api.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/script/api/math.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/scene.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    /*
     * Class:     dev_lunar_core_GameObject
     * Method:    _getPosition
     * Signature: ()[F
     */
    JNIEXPORT jdoubleArray JNICALL Java_dev_lunar_core_GameObject__1getPosition
    (JNIEnv* env, jobject object)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        auto& transform = game_object.getTransform();
        jdoubleArray array = env->NewDoubleArray(3);
        env->SetDoubleArrayRegion(array, 0, 3, (jdouble*)glm::value_ptr(transform.position));
        return array;
    }

    /*
     * Class:     dev_lunar_core_GameObject
     * Method:    _getRotation
     * Signature: ()[F
     */
    JNIEXPORT jdoubleArray JNICALL Java_dev_lunar_core_GameObject__1getRotation
    (JNIEnv* env, jobject object)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        auto& transform = game_object.getTransform();
        jdoubleArray array = env->NewDoubleArray(3);
        env->SetDoubleArrayRegion(array, 0, 3, (jdouble*)glm::value_ptr(transform.rotation));
        return array;
    }

    /*
     * Class:     dev_lunar_core_GameObject
     * Method:    _getScale
     * Signature: ()[F
     */
    JNIEXPORT jdoubleArray JNICALL Java_dev_lunar_core_GameObject__1getScale
    (JNIEnv* env, jobject object)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        auto& transform = game_object.getTransform();
        jdoubleArray array = env->NewDoubleArray(3);
        env->SetDoubleArrayRegion(array, 0, 3, (jdouble*)glm::value_ptr(transform.scale));
        return array;
    }

    /*
     * Class:     dev_lunar_core_GameObject
     * Method:    setPosition
     * Signature: (FFF)V
     */
    JNIEXPORT void JNICALL Java_dev_lunar_core_GameObject_setPosition
    (JNIEnv*, jobject object, jdouble x, jdouble y, jdouble z)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        auto& transform = game_object.getTransform();
        transform.position = { x, y, z };
    }

    /*
     * Class:     dev_lunar_core_GameObject
     * Method:    setRotation
     * Signature: (FFF)V
     */
    JNIEXPORT void JNICALL Java_dev_lunar_core_GameObject_setRotation
    (JNIEnv*, jobject object, jdouble x, jdouble y, jdouble z)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        auto& transform = game_object.getTransform();
        transform.rotation = { x, y, z };
    }

    /*
     * Class:     dev_lunar_core_GameObject
     * Method:    setScale
     * Signature: (FFF)V
     */
    JNIEXPORT void JNICALL Java_dev_lunar_core_GameObject_setScale
    (JNIEnv*, jobject object, jdouble x, jdouble y, jdouble z)
    {
        auto& game_object = Script::getGameObjectFromHandle(object);
        auto& transform = game_object.getTransform();
        transform.scale = { x, y, z };
    }
}
