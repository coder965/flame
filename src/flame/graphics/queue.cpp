#include "queue_private.h"

namespace flame
{
	namespace graphics
	{
		static int queue_count = 1;

		Queue *create_queue(Graphics *g)
		{
			assert(queue_count > 0);

			auto q = new Queue;
			q->_priv = new QueuePrivate;
			vkGetDeviceQueue(g->_priv->device, 0, 0, &q->_priv->v);

			queue_count--;

			return q;
		}

		void destroy_queue(Graphics *g, Queue *q)
		{
			delete q->_priv;
			delete q;

			queue_count++;;
		}
	}
}
