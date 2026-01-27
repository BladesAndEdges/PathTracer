#include "Material4Index.h"

// --------------------------------------------------------------------------------
Material4Index::Material4Index()
{
	for (uint32_t index = 0u; index < 4u; index++)
	{
		m_indices[index] = UINT32_MAX;
	}
}
