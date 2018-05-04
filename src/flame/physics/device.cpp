#include "device_private.h"

namespace flame
{
	namespace physics
	{
		Device *create_device()
		{
			auto d = new Device;
			
			d->_priv = new DevicePrivate;

			return d;
		}

		void destroy_device(Device *d)
		{
			delete d->_priv;
			delete d;
		}
	}
}

