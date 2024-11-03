#include <lunar/script/script_api.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <jni.h>

extern "C"
{
	///*
	// * Class:     dev_lunar_core_components_Transform
	// * Method:    getPositionA
	// * Signature: ()[D
	// */
	//JNIEXPORT jdoubleArray JNICALL Java_dev_lunar_core_components_Transform_getPositionA
	//(JNIEnv* env, jobject object)
	//{
	//	auto& vm = Script::getVmFromEnv(env);
	//	
	//	auto object_id = vm.getNativeHandle(object);
	//	auto& transform = Core::getActiveScene()
	//							.getGameObject(object_id)
	//							.getTransform();
	//	
	//	jdoubleArray pos = env->NewDoubleArray(3);
	//	env->SetDoubleArrayRegion(pos, 0, 3, glm::value_ptr(transform.position));
	//	return pos;
	//}

	///*
	// * Class:     dev_lunar_core_components_Transform
	// * Method:    getRotationA
	// * Signature: ()[D
	// */
	//JNIEXPORT jdoubleArray JNICALL Java_dev_lunar_core_components_Transform_getRotationA
	//(JNIEnv* env, jobject object)
	//{
	//	auto& vm = Script::getVmFromEnv(env);

	//	auto object_id = vm.getNativeHandle(object);
	//	auto& transform = Core::getActiveScene()
	//		.getGameObject(object_id)
	//		.getTransform();

	//	jdoubleArray rotation = env->NewDoubleArray(3);
	//	env->SetDoubleArrayRegion(rotation, 0, 3, glm::value_ptr(transform.rotation));
	//	return rotation;
	//}

	///*
	// * Class:     dev_lunar_core_components_Transform
	// * Method:    getScaleA
	// * Signature: ()[D
	// */
	//JNIEXPORT jdoubleArray JNICALL Java_dev_lunar_core_components_Transform_getScaleA
	//(JNIEnv* env, jobject object)
	//{
	//	auto& vm = Script::getVmFromEnv(env);

	//	auto object_id = vm.getNativeHandle(object);
	//	auto& transform = Core::getActiveScene()
	//		.getGameObject(object_id)
	//		.getTransform();

	//	jdoubleArray scale = env->NewDoubleArray(3);
	//	env->SetDoubleArrayRegion(scale, 0, 3, glm::value_ptr(transform.scale));
	//	return scale;
	//}

	///*
	// * Class:     dev_lunar_core_components_Transform
	// * Method:    setPositionD
	// * Signature: (DDD)V
	// */
	//JNIEXPORT void JNICALL Java_dev_lunar_core_components_Transform_setPositionD
	//(JNIEnv*, jobject, jdouble, jdouble, jdouble);

	///*
	// * Class:     dev_lunar_core_components_Transform
	// * Method:    setRotationD
	// * Signature: (DDD)V
	// */
	//JNIEXPORT void JNICALL Java_dev_lunar_core_components_Transform_setRotationD
	//(JNIEnv*, jobject, jdouble, jdouble, jdouble);

	///*
	// * Class:     dev_lunar_core_components_Transform
	// * Method:    setScaleD
	// * Signature: (DDD)V
	// */
	//JNIEXPORT void JNICALL Java_dev_lunar_core_components_Transform_setScaleD
	//(JNIEnv*, jobject, jdouble, jdouble, jdouble);
}
