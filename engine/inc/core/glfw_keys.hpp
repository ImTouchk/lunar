#pragma once
#include <GLFW/glfw3.h>

enum class CKey
{
	eAny = 0,
	eLeftShift  = GLFW_KEY_LEFT_SHIFT,
	eRightShift = GLFW_KEY_RIGHT_SHIFT,

	eLeftAlt  = GLFW_KEY_LEFT_ALT,     eRightAlt  = GLFW_KEY_RIGHT_ALT,
	eLeftCtrl = GLFW_KEY_LEFT_CONTROL, eRightCtrl = GLFW_KEY_RIGHT_CONTROL,
	eLeftFn   = GLFW_KEY_LEFT_SUPER,   eRightFn   = GLFW_KEY_RIGHT_SUPER,

	eF1  = GLFW_KEY_F1,  eF2  = GLFW_KEY_F2,
	eF3  = GLFW_KEY_F3,  eF4  = GLFW_KEY_F4,
	eF5  = GLFW_KEY_F5,  eF6  = GLFW_KEY_F6,
	eF7  = GLFW_KEY_F7,  eF8  = GLFW_KEY_F8,
	eF9  = GLFW_KEY_F9,  eF10 = GLFW_KEY_F10,
	eF11 = GLFW_KEY_F11, eF12 = GLFW_KEY_F12,

	eNum0 = GLFW_KEY_0, eNum1 = GLFW_KEY_1,
	eNum2 = GLFW_KEY_2, eNum3 = GLFW_KEY_3,
	eNum4 = GLFW_KEY_4, eNum5 = GLFW_KEY_5,
	eNum6 = GLFW_KEY_6, eNum7 = GLFW_KEY_7,
	eNum8 = GLFW_KEY_8, eNum9 = GLFW_KEY_9,

	eQ = GLFW_KEY_Q, eW = GLFW_KEY_W, eE = GLFW_KEY_E, eR = GLFW_KEY_R,
	eT = GLFW_KEY_T, eY = GLFW_KEY_Y, eU = GLFW_KEY_U, eI = GLFW_KEY_I,
	eO = GLFW_KEY_O, eP = GLFW_KEY_P, eA = GLFW_KEY_A, eS = GLFW_KEY_S,
	eD = GLFW_KEY_D, eF = GLFW_KEY_F, eG = GLFW_KEY_G, eH = GLFW_KEY_H,
	eJ = GLFW_KEY_J, eK = GLFW_KEY_K, eL = GLFW_KEY_L, eZ = GLFW_KEY_Z,
	eX = GLFW_KEY_X, eC = GLFW_KEY_C, eV = GLFW_KEY_V, eB = GLFW_KEY_B,
	eN = GLFW_KEY_N, eM = GLFW_KEY_M,

	eDot       = GLFW_KEY_PERIOD,
	eComma     = GLFW_KEY_COMMA,
	eSemicolon = GLFW_KEY_SEMICOLON,
	eEqual     = GLFW_KEY_EQUAL,
	eMinus     = GLFW_KEY_MINUS,
	ePlus      = GLFW_KEY_KP_ADD,
	eStar      = GLFW_KEY_KP_MULTIPLY,
	eSlash     = GLFW_KEY_SLASH,
	eBackslash = GLFW_KEY_BACKSLASH,
	eBackSpace = GLFW_KEY_BACKSPACE,
	eSpace     = GLFW_KEY_SPACE,
};
