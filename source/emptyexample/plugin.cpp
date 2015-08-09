#include <gloperate/plugin/plugin_api.h>

#include "PhysicallyBasedRenderingExample.h"

#include <glexamples-version.h>

GLOPERATE_PLUGIN_LIBRARY

    GLOPERATE_PLUGIN(EmptyExample
    , "PBR"
    , "Copy to implement a new example"
    , GLEXAMPLES_AUTHOR_ORGANIZATION
    , "v1.0.0" )

GLOPERATE_PLUGIN_LIBRARY_END
