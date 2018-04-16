#include <vector>
#include <assert.h>

#include <flame/global.h>
#include <flame/system.h>

#include "graphics.h"
#include "graphics_private.h"

namespace flame
{
	const char *vk_device_type_names[] = {
		"other",
		"integrated gpu",
		"discrete gpu",
		"virtual gpu",
		"cpu"
	};
}
