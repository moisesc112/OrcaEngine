#pragma once

#include <OrcaEngine/ECS/Registry.hpp>
#include <OrcaEngine/Rendering/RenderTypes.hpp>

namespace RenderExtraction{

    RenderBundle ExtractRenderBundle(Registry& registry);
    
}
