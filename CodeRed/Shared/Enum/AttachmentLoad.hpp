#pragma once

#ifdef __CODE__RED__GLOBAL__INCLUDE__
#include <Shared/Utility.hpp>
#else
#include "../Utility.hpp"
#endif

namespace CodeRed {

	enum class AttachmentLoad : UInt32 {
		Clear,
		Load,
		DontCare
	};
	
}