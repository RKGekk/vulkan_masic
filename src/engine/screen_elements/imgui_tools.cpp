#include "imgui_tools.h"

#include "../../scene/nodes/basic_camera_node.h"
#include "../../actors/transform_component.h"

std::string getPrimitiveTopologyStr(VkPrimitiveTopology topology) {
    switch (topology) {
        case VK_PRIMITIVE_TOPOLOGY_POINT_LIST : return "POINT_LIST";
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST : return "LINE_LIST";
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : return "LINE_STRIP";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : return "TRIANGLE_LIST";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : return "TRIANGLE_STRIP";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN : return "TRIANGLE_FAN";
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY : return "LINE_LIST_WITH_ADJACENCY";
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY : return "LINE_STRIP_WITH_ADJACENCY";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY : return "TRIANGLE_LIST_WITH_ADJACENCY";
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY : return "TRIANGLE_STRIP_WITH_ADJACENCY";
        case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : return "PATCH_LIST";
        default: return "TRIANGLE_LIST";
    }
}

std::string getFormatStr(VkFormat format) {
    switch (format) {
        case VK_FORMAT_UNDEFINED  : return "UNDEFINED";
        case VK_FORMAT_R4G4_UNORM_PACK8  : return "R4G4_UNORM_PACK8";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16  : return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16  : return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R5G6B5_UNORM_PACK16  : return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_B5G6R5_UNORM_PACK16  : return "B5G6R5_UNORM_PACK16";
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16  : return "R5G5B5A1_UNORM_PACK16";
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16  : return "B5G5R5A1_UNORM_PACK16";
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16  : return "A1R5G5B5_UNORM_PACK16";
        case VK_FORMAT_R8_UNORM  : return "R8_UNORM";
        case VK_FORMAT_R8_SNORM  : return "R8_SNORM";
        case VK_FORMAT_R8_USCALED  : return "R8_USCALED";
        case VK_FORMAT_R8_SSCALED  : return "R8_SSCALED";
        case VK_FORMAT_R8_UINT  : return "R8_UINT";
        case VK_FORMAT_R8_SINT  : return "R8_SINT";
        case VK_FORMAT_R8_SRGB  : return "R8_SRGB";
        case VK_FORMAT_R8G8_UNORM  : return "R8G8_UNORM";
        case VK_FORMAT_R8G8_SNORM  : return "R8G8_SNORM";
        case VK_FORMAT_R8G8_USCALED  : return "R8G8_USCALED";
        case VK_FORMAT_R8G8_SSCALED  : return "R8G8_SSCALED";
        case VK_FORMAT_R8G8_UINT  : return "R8G8_UINT";
        case VK_FORMAT_R8G8_SINT  : return "R8G8_SINT";
        case VK_FORMAT_R8G8_SRGB  : return "R8G8_SRGB";
        case VK_FORMAT_R8G8B8_UNORM  : return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8B8_SNORM  : return "R8G8B8_SNORM";
        case VK_FORMAT_R8G8B8_USCALED  : return "R8G8B8_USCALED";
        case VK_FORMAT_R8G8B8_SSCALED  : return "R8G8B8_SSCALED";
        case VK_FORMAT_R8G8B8_UINT  : return "R8G8B8_UINT";
        case VK_FORMAT_R8G8B8_SINT  : return "R8G8B8_SINT";
        case VK_FORMAT_R8G8B8_SRGB  : return "R8G8B8_SRGB";
        case VK_FORMAT_B8G8R8_UNORM  : return "B8G8R8_UNORM";
        case VK_FORMAT_B8G8R8_SNORM  : return "B8G8R8_SNORM";
        case VK_FORMAT_B8G8R8_USCALED  : return "B8G8R8_USCALED";
        case VK_FORMAT_B8G8R8_SSCALED  : return "B8G8R8_SSCALED";
        case VK_FORMAT_B8G8R8_UINT  : return "B8G8R8_UINT";
        case VK_FORMAT_B8G8R8_SINT  : return "B8G8R8_SINT";
        case VK_FORMAT_B8G8R8_SRGB  : return "B8G8R8_SRGB";
        case VK_FORMAT_R8G8B8A8_UNORM  : return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_SNORM  : return "R8G8B8A8_SNORM";
        case VK_FORMAT_R8G8B8A8_USCALED  : return "R8G8B8A8_USCALED";
        case VK_FORMAT_R8G8B8A8_SSCALED  : return "R8G8B8A8_SSCALED";
        case VK_FORMAT_R8G8B8A8_UINT  : return "R8G8B8A8_UINT";
        case VK_FORMAT_R8G8B8A8_SINT  : return "R8G8B8A8_SINT";
        case VK_FORMAT_R8G8B8A8_SRGB  : return "R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM  : return "B8G8R8A8_UNORM";
        case VK_FORMAT_B8G8R8A8_SNORM  : return "B8G8R8A8_SNORM";
        case VK_FORMAT_B8G8R8A8_USCALED  : return "B8G8R8A8_USCALED";
        case VK_FORMAT_B8G8R8A8_SSCALED  : return "B8G8R8A8_SSCALED";
        case VK_FORMAT_B8G8R8A8_UINT  : return "B8G8R8A8_UINT";
        case VK_FORMAT_B8G8R8A8_SINT  : return "B8G8R8A8_SINT";
        case VK_FORMAT_B8G8R8A8_SRGB  : return "B8G8R8A8_SRGB";
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32  : return "A8B8G8R8_UNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32  : return "A8B8G8R8_SNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32  : return "A8B8G8R8_USCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32  : return "A8B8G8R8_SSCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_UINT_PACK32  : return "A8B8G8R8_UINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SINT_PACK32  : return "A8B8G8R8_SINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32  : return "A8B8G8R8_SRGB_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32  : return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32  : return "A2R10G10B10_SNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32  : return "A2R10G10B10_USCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32  : return "A2R10G10B10_SSCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_UINT_PACK32  : return "A2R10G10B10_UINT_PACK32";
        case VK_FORMAT_A2R10G10B10_SINT_PACK32  : return "A2R10G10B10_SINT_PACK32";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32  : return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32  : return "A2B10G10R10_SNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32  : return "A2B10G10R10_USCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32  : return "A2B10G10R10_SSCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_UINT_PACK32  : return "A2B10G10R10_UINT_PACK32";
        case VK_FORMAT_A2B10G10R10_SINT_PACK32  : return "A2B10G10R10_SINT_PACK32";
        case VK_FORMAT_R16_UNORM  : return "R16_UNORM";
        case VK_FORMAT_R16_SNORM  : return "R16_SNORM";
        case VK_FORMAT_R16_USCALED  : return "R16_USCALED";
        case VK_FORMAT_R16_SSCALED  : return "R16_SSCALED";
        case VK_FORMAT_R16_UINT  : return "R16_UINT";
        case VK_FORMAT_R16_SINT  : return "R16_SINT";
        case VK_FORMAT_R16_SFLOAT  : return "R16_SFLOAT";
        case VK_FORMAT_R16G16_UNORM  : return "R16G16_UNORM";
        case VK_FORMAT_R16G16_SNORM  : return "R16G16_SNORM";
        case VK_FORMAT_R16G16_USCALED  : return "R16G16_USCALED";
        case VK_FORMAT_R16G16_SSCALED  : return "R16G16_SSCALED";
        case VK_FORMAT_R16G16_UINT  : return "R16G16_UINT";
        case VK_FORMAT_R16G16_SINT  : return "R16G16_SINT";
        case VK_FORMAT_R16G16_SFLOAT  : return "R16G16_SFLOAT";
        case VK_FORMAT_R16G16B16_UNORM  : return "R16G16B16_UNORM";
        case VK_FORMAT_R16G16B16_SNORM  : return "R16G16B16_SNORM";
        case VK_FORMAT_R16G16B16_USCALED  : return "R16G16B16_USCALED";
        case VK_FORMAT_R16G16B16_SSCALED  : return "R16G16B16_SSCALED";
        case VK_FORMAT_R16G16B16_UINT  : return "R16G16B16_UINT";
        case VK_FORMAT_R16G16B16_SINT  : return "R16G16B16_SINT";
        case VK_FORMAT_R16G16B16_SFLOAT  : return "R16G16B16_SFLOAT";
        case VK_FORMAT_R16G16B16A16_UNORM  : return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16B16A16_SNORM  : return "R16G16B16A16_SNORM";
        case VK_FORMAT_R16G16B16A16_USCALED  : return "R16G16B16A16_USCALED";
        case VK_FORMAT_R16G16B16A16_SSCALED  : return "R16G16B16A16_SSCALED";
        case VK_FORMAT_R16G16B16A16_UINT  : return "R16G16B16A16_UINT";
        case VK_FORMAT_R16G16B16A16_SINT  : return "R16G16B16A16_SINT";
        case VK_FORMAT_R16G16B16A16_SFLOAT  : return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R32_UINT  : return "R32_UINT";
        case VK_FORMAT_R32_SINT  : return "R32_SINT";
        case VK_FORMAT_R32_SFLOAT  : return "R32_SFLOAT";
        case VK_FORMAT_R32G32_UINT  : return "R32G32_UINT";
        case VK_FORMAT_R32G32_SINT  : return "R32G32_SINT";
        case VK_FORMAT_R32G32_SFLOAT  : return "R32G32_SFLOAT";
        case VK_FORMAT_R32G32B32_UINT  : return "R32G32B32_UINT";
        case VK_FORMAT_R32G32B32_SINT  : return "R32G32B32_SINT";
        case VK_FORMAT_R32G32B32_SFLOAT  : return "R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32B32A32_UINT  : return "R32G32B32A32_UINT";
        case VK_FORMAT_R32G32B32A32_SINT  : return "R32G32B32A32_SINT";
        case VK_FORMAT_R32G32B32A32_SFLOAT  : return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R64_UINT  : return "R64_UINT";
        case VK_FORMAT_R64_SINT  : return "R64_SINT";
        case VK_FORMAT_R64_SFLOAT  : return "R64_SFLOAT";
        case VK_FORMAT_R64G64_UINT  : return "R64G64_UINT";
        case VK_FORMAT_R64G64_SINT  : return "R64G64_SINT";
        case VK_FORMAT_R64G64_SFLOAT  : return "R64G64_SFLOAT";
        case VK_FORMAT_R64G64B64_UINT  : return "R64G64B64_UINT";
        case VK_FORMAT_R64G64B64_SINT  : return "R64G64B64_SINT";
        case VK_FORMAT_R64G64B64_SFLOAT  : return "R64G64B64_SFLOAT";
        case VK_FORMAT_R64G64B64A64_UINT  : return "R64G64B64A64_UINT";
        case VK_FORMAT_R64G64B64A64_SINT  : return "R64G64B64A64_SINT";
        case VK_FORMAT_R64G64B64A64_SFLOAT  : return "R64G64B64A64_SFLOAT";
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32  : return "B10G11R11_UFLOAT_PACK32";
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32  : return "E5B9G9R9_UFLOAT_PACK32";
        case VK_FORMAT_D16_UNORM  : return "D16_UNORM";
        case VK_FORMAT_X8_D24_UNORM_PACK32  : return "X8_D24_UNORM_PACK32";
        case VK_FORMAT_D32_SFLOAT  : return "D32_SFLOAT";
        case VK_FORMAT_S8_UINT  : return "S8_UINT";
        case VK_FORMAT_D16_UNORM_S8_UINT  : return "D16_UNORM_S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT  : return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT  : return "D32_SFLOAT_S8_UINT";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK  : return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK  : return "BC1_RGB_SRGB_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK  : return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK  : return "BC1_RGBA_SRGB_BLOCK";
        case VK_FORMAT_BC2_UNORM_BLOCK  : return "BC2_UNORM_BLOCK";
        case VK_FORMAT_BC2_SRGB_BLOCK  : return "BC2_SRGB_BLOCK";
        case VK_FORMAT_BC3_UNORM_BLOCK  : return "BC3_UNORM_BLOCK";
        case VK_FORMAT_BC3_SRGB_BLOCK  : return "BC3_SRGB_BLOCK";
        case VK_FORMAT_BC4_UNORM_BLOCK  : return "BC4_UNORM_BLOCK";
        case VK_FORMAT_BC4_SNORM_BLOCK  : return "BC4_SNORM_BLOCK";
        case VK_FORMAT_BC5_UNORM_BLOCK  : return "BC5_UNORM_BLOCK";
        case VK_FORMAT_BC5_SNORM_BLOCK  : return "BC5_SNORM_BLOCK";
        case VK_FORMAT_BC6H_UFLOAT_BLOCK  : return "BC6H_UFLOAT_BLOCK";
        case VK_FORMAT_BC6H_SFLOAT_BLOCK  : return "BC6H_SFLOAT_BLOCK";
        case VK_FORMAT_BC7_UNORM_BLOCK  : return "BC7_UNORM_BLOCK";
        case VK_FORMAT_BC7_SRGB_BLOCK  : return "BC7_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK  : return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK  : return "ETC2_R8G8B8_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK  : return "ETC2_R8G8B8A1_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK  : return "ETC2_R8G8B8A1_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK  : return "ETC2_R8G8B8A8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK  : return "ETC2_R8G8B8A8_SRGB_BLOCK";
        case VK_FORMAT_EAC_R11_UNORM_BLOCK  : return "EAC_R11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11_SNORM_BLOCK  : return "EAC_R11_SNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK  : return "EAC_R11G11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK  : return "EAC_R11G11_SNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK  : return "ASTC_4x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK  : return "ASTC_4x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK  : return "ASTC_5x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK  : return "ASTC_5x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK  : return "ASTC_5x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK  : return "ASTC_5x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK  : return "ASTC_6x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK  : return "ASTC_6x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK  : return "ASTC_6x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK  : return "ASTC_6x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK  : return "ASTC_8x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK  : return "ASTC_8x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK  : return "ASTC_8x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK  : return "ASTC_8x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK  : return "ASTC_8x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK  : return "ASTC_8x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK  : return "ASTC_10x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK  : return "ASTC_10x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK  : return "ASTC_10x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK  : return "ASTC_10x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK  : return "ASTC_10x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK  : return "ASTC_10x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK  : return "ASTC_10x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK  : return "ASTC_10x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK  : return "ASTC_12x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK  : return "ASTC_12x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK  : return "ASTC_12x12_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK  : return "ASTC_12x12_SRGB_BLOCK";
        case VK_FORMAT_G8B8G8R8_422_UNORM  : return "G8B8G8R8_422_UNORM";
        case VK_FORMAT_B8G8R8G8_422_UNORM  : return "B8G8R8G8_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM  : return "G8_B8_R8_3PLANE_420_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM  : return "G8_B8R8_2PLANE_420_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM  : return "G8_B8_R8_3PLANE_422_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM  : return "G8_B8R8_2PLANE_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM  : return "G8_B8_R8_3PLANE_444_UNORM";
        case VK_FORMAT_R10X6_UNORM_PACK16  : return "R10X6_UNORM_PACK16";
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16  : return "R10X6G10X6_UNORM_2PACK16";
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16  : return "R10X6G10X6B10X6A10X6_UNORM_4PACK16";
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16  : return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16  : return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16  : return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16  : return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16  : return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16  : return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16  : return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_R12X4_UNORM_PACK16  : return "R12X4_UNORM_PACK16";
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16  : return "R12X4G12X4_UNORM_2PACK16";
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16  : return "R12X4G12X4B12X4A12X4_UNORM_4PACK16";
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16  : return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16  : return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16  : return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16  : return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16  : return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16  : return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16  : return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16B16G16R16_422_UNORM  : return "G16B16G16R16_422_UNORM";
        case VK_FORMAT_B16G16R16G16_422_UNORM  : return "B16G16R16G16_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM  : return "G16_B16_R16_3PLANE_420_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM  : return "G16_B16R16_2PLANE_420_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM  : return "G16_B16_R16_3PLANE_422_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM  : return "G16_B16R16_2PLANE_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM  : return "G16_B16_R16_3PLANE_444_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM  : return "G8_B8R8_2PLANE_444_UNORM";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16  : return "G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16  : return "G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM  : return "G16_B16R16_2PLANE_444_UNORM";
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16  : return "A4R4G4B4_UNORM_PACK16";
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16  : return "A4B4G4R4_UNORM_PACK16";
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK  : return "ASTC_4x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK  : return "ASTC_5x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK  : return "ASTC_5x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK  : return "ASTC_6x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK  : return "ASTC_6x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK  : return "ASTC_8x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK  : return "ASTC_8x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK  : return "ASTC_8x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK  : return "ASTC_10x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK  : return "ASTC_10x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK  : return "ASTC_10x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK  : return "ASTC_10x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK  : return "ASTC_12x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK  : return "ASTC_12x12_SFLOAT_BLOCK";
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG  : return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG  : return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG  : return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG  : return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG  : return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG  : return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG  : return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG  : return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_R16G16_SFIXED5_NV  : return "R16G16_SFIXED5_NV";
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR  : return "A1B5G5R5_UNORM_PACK16_KHR";
        case VK_FORMAT_A8_UNORM_KHR  : return "A8_UNORM_KHR";
        default: return "UNDEFINED";
    }
}

std::string getMemoryPropertyStr(VkMemoryPropertyFlags prop) {
    using namespace std::literals;
    std::string res = ""s;
    if (prop & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) res += "DEVICE_LOCAL"s;
    if (prop & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) res += "/HOST_VISIBLE"s;
    if (prop & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) res += "/HOST_COHERENT"s;
    if (prop & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) res += "/HOST_CACHED"s;
    if (prop & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) res += "/LAZILY_ALLOCATED"s;
    if (prop & VK_MEMORY_PROPERTY_PROTECTED_BIT) res += "/PROTECTED"s;
    if (prop & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) res += "/DEVICE_COHERENT_AMD"s;
    if (prop & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) res += "/DEVICE_UNCACHED_AMD"s;
    
    return res;
}

std::string getBufferUsageStr(VkBufferUsageFlags buff_usage) {
    using namespace std::literals;
    std::string res = ""s;
    if(buff_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) res += "TRANSFER_SRC"s;
    if(buff_usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) res += "/TRANSFER_DST"s;
    if(buff_usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) res += "/UNIFORM_TEXEL_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) res += "/STORAGE_TEXEL_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) res += "/UNIFORM_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) res += "/STORAGE_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) res += "/INDEX_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) res += "/VERTEX_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) res += "/INDIRECT_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) res += "/SHADER_DEVICE_ADDRESS"s;
    if(buff_usage & VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR) res += "/VIDEO_DECODE_SRC"s;
    if(buff_usage & VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR) res += "/VIDEO_DECODE_DST"s;
    if(buff_usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT) res += "/TRANSFORM_FEEDBACK_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) res += "/TRANSFORM_FEEDBACK_COUNTER_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT) res += "/CONDITIONAL_RENDERING"s;
    if(buff_usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) res += "/ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY"s;
    if(buff_usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) res += "/ACCELERATION_STRUCTURE_STORAGE"s;
    if(buff_usage & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR) res += "/SHADER_BINDING_TABLE"s;
    if(buff_usage & VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR) res += "/VIDEO_ENCODE_DST"s;
    if(buff_usage & VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) res += "/VIDEO_ENCODE_SRC"s;
    if(buff_usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) res += "/SAMPLER_DESCRIPTOR_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) res += "/RESOURCE_DESCRIPTOR_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) res += "/PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER"s;
    if(buff_usage & VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT) res += "/MICROMAP_BUILD_INPUT_READ_ONLY"s;
    if(buff_usage & VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT) res += "/MICROMAP_STORAGE"s;

    return res;
}

std::string getMaterialTextures(uint32_t has_texture) {
    using namespace std::literals;
    std::string res = ""s;

    if(has_texture & Material::HAS_AMBIENT_TEXTURE) res += "AMBIENT";
    if(has_texture & Material::HAS_EMISSIVE_TEXTURE) res += "EMISSIVE";
    if(has_texture & Material::HAS_DIFFUSE_TEXTURE) res += "DIFFUSE";
    if(has_texture & Material::HAS_SPECULAR_TEXTURE) res += "SPECULAR";
    if(has_texture & Material::HAS_SPECULAR_POWER_TEXTURE) res += "SPECULAR_POWER";
    if(has_texture & Material::HAS_NORMAL_TEXTURE) res += "NORMAL";
    if(has_texture & Material::HAS_BUMP_TEXTURE) res += "BUMP";
    if(has_texture & Material::HAS_OPACITY_TEXTURE) res += "OPACITY";
    if(has_texture & Material::HAS_DISPLACEMENT_TEXTURE) res += "DISPLACEMENT";
    if(has_texture & Material::HAS_METALNESS_TEXTURE) res += "METALNESS";
    if(has_texture & Material::HAS_NORMAL_INV_Y_TEXTURE) res += "NORMAL_INV_Y";
    if(has_texture & Material::HAS_SHADOW_TEXTURE) res += "SHADOW";

    return res;
}

std::string getNodeFlagsStr(Scene::NodeTypeFlags node_type_flags) {
    using namespace std::literals;
    std::string node_type_flags_str = "f"s;
    if(!node_type_flags) node_type_flags_str += "/N"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_MESH) node_type_flags_str += "/M"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_LIGHT) node_type_flags_str += "/L"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_CAMERA) node_type_flags_str += "/C"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_SHADOW_CAMERA) node_type_flags_str += "/Sh"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_AABB) node_type_flags_str += "/AABB"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_SPHERE) node_type_flags_str += "/Sp"s;
    if(node_type_flags & Scene::NODE_TYPE_FLAG_BONE) node_type_flags_str += "/B"s;
    return node_type_flags_str;
}

std::string getSummaryForHierarchyStr(Scene::NodeIndex node_index, Scene::Hierarchy hierarchy_node, Scene::NodeTypeFlags node_type_flags, std::string node_name) {
    using namespace std::literals;
    std::string node_type_flags_str = getNodeFlagsStr(node_type_flags);

    std::string header = "i"s + std::to_string(node_index)
                       + " p"s + (hierarchy_node.parent != Scene::NO_INDEX ? std::to_string(hierarchy_node.parent) : "N"s)
                       + " c"s + (hierarchy_node.first_child != Scene::NO_INDEX ? std::to_string(hierarchy_node.first_child) : "N"s)
                       + " s"s + (hierarchy_node.next_sibling != Scene::NO_INDEX ? std::to_string(hierarchy_node.next_sibling) : "N"s)
                       + " l"s + std::to_string(hierarchy_node.level)
                       + " "s + node_type_flags_str
                       + " "s + node_name;

    return header;
}

void printQuatImGUI(glm::quat q) {
    glm::vec3 pyr_xm = glm::eulerAngles(q);
	glm::vec3 ypr_xm(
		glm::degrees(pyr_xm.y),
		glm::degrees(pyr_xm.x),
		glm::degrees(pyr_xm.z)
	);

    if (ImGui::InputFloat4("Rq", ((float*)&q), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat3("Ypr", ((float*)&ypr_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
}

void printMatrixImGUI(const glm::mat4& matrix) {
    if (ImGui::InputFloat4("R1", ((float*)&matrix) + 0, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat4("R2", ((float*)&matrix) + 4, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat4("R3", ((float*)&matrix) + 8, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat4("R4", ((float*)&matrix) + 12, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}

    if(ImGui::TreeNode("DecomposedMatrix")) {
        printDecomposedMatrixImGUI(matrix);
        ImGui::TreePop();
    }
}

void editMatrixImGUI(const glm::mat4& mat, std::function<void(glm::bvec3 tsr, glm::vec3 tr, glm::vec3 sc, glm::quat rot)> fn) {
    
    glm::mat4 matrix = mat;
	glm::vec3 translation_xm = matrix[3];
    glm::vec3 scale_xm;
    for (int i = 0; i < 3; ++i) {
        scale_xm[i] = glm::length(matrix[i]);
        matrix[i] /= scale_xm[i];
    }
    glm::quat rotation_xm = glm::toQuat(matrix);
	
	printQuatImGUI(rotation_xm);
	if (ImGui::InputFloat3("Sc", ((float*)&scale_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
	if (ImGui::SliderFloat3("Tr", ((float*)&translation_xm), -8.0f, 8.0f)) {
        fn({true, false, false}, translation_xm, scale_xm, rotation_xm);
    }

    float dx = 0.0f;
	float dy = 0.0f;
    float dz = 0.0f;
    float step = 0.1f;
    bool change = false;
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

    ImGui::PushID("Rotate X");
    ImGui::Text("Rotate X");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        dx += step;
        change = true;
    };
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        dx -= step;
        change = true;
    }
    ImGui::PopID();

    ImGui::SameLine();

    ImGui::PushID("Rotate Y");
    ImGui::Text("Rotate Y");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        dy += step;
        change = true;
    };
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        dy -= step;
        change = true;
    }
    ImGui::PopID();

    ImGui::SameLine();

    ImGui::PushID("Rotate Z");
    ImGui::Text("Rotate Z");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        dz += step;
        change = true;
    };
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        dz -= step;
        change = true;
    }
    ImGui::PopID();

	if(change) {
        glm::quat q1 = glm::angleAxis(dx, TransformComponent::GetDefaultUp3f());
		glm::quat q2 = glm::angleAxis(dy, TransformComponent::GetDefaultRight3f());
        glm::quat q3 = glm::angleAxis(dz, TransformComponent::GetDefaultForward3f());
		glm::quat q4 = glm::normalize(rotation_xm * q1 * q2 * q3);

        fn(
            {false, false, true},
            translation_xm,
            scale_xm,
            q4
        );
	}
}

void printDecomposedMatrixImGUI(glm::mat4 matrix) {
    glm::vec3 scale_xm;
	glm::vec3 translation_xm;

    translation_xm = matrix[3];

    for (int i = 0; i < 3; ++i) {
        scale_xm[i] = glm::length(matrix[i]);
        matrix[i] /= scale_xm[i];
    }
                        
    printQuatImGUI(glm::toQuat(matrix));

	if (ImGui::InputFloat3("Sc", ((float*)&scale_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
	if (ImGui::InputFloat3("Tr", ((float*)&translation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
}

void printBoundingBoxImGUI(const BoundingBox& bounding_box) {
    if (ImGui::InputFloat3("Center", ((float*)&bounding_box.Center), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat3("Extents", ((float*)&bounding_box.Extents), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
}

void printBoundingSphereImGUI(const BoundingSphere& bounding_sphere) {
    if (ImGui::InputFloat3("Center", ((float*)&bounding_sphere.Center), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat("Radius", const_cast<float*>(&bounding_sphere.Radius), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
}

void printBoundingFrustumImGUI(const BoundingFrustum& bounding_frustum) {
    if (ImGui::InputFloat3("Origin", ((float*)&bounding_frustum.Origin), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    printQuatImGUI(bounding_frustum.Orientation);
    if (ImGui::InputFloat("RightSlope", const_cast<float*>(&bounding_frustum.RightSlope), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat("LeftSlope", const_cast<float*>(&bounding_frustum.LeftSlope), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat("TopSlope", const_cast<float*>(&bounding_frustum.TopSlope), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat("BottomSlope", const_cast<float*>(&bounding_frustum.BottomSlope), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat("Near", const_cast<float*>(&bounding_frustum.Near), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
    if (ImGui::InputFloat("Far", const_cast<float*>(&bounding_frustum.Far), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
}

void printVulkanBufferImGUI(std::shared_ptr<VulkanBuffer> vk_buffer) {
    int vtx_buff_sz = vk_buffer->getSize();
    ImGui::InputInt("BufferSize", &vtx_buff_sz, 0, 0, ImGuiInputTextFlags_ReadOnly);

    std::string vtx_mem_prop_str = getMemoryPropertyStr(vk_buffer->getProperties());
    ImGui::InputText("BufferMemoryProperty", const_cast<char*>(vtx_mem_prop_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

    std::string vtx_usage_str = getBufferUsageStr(vk_buffer->getUsage());
    ImGui::InputText("BufferUsage", const_cast<char*>(vtx_usage_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
}

void printVertexBufferImGUI(std::shared_ptr<VertexBuffer> vtx, VkPrimitiveTopology topology) {
    using namespace std::literals;
    if(!vtx) return;

    std::string topology_str = getPrimitiveTopologyStr(topology);
    ImGui::InputText("PrimitiveTopology", const_cast<char*>(topology_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

    int index_count = vtx->getIndicesCount();
	ImGui::InputInt("IndexCount", &index_count, 0, 0, ImGuiInputTextFlags_ReadOnly);

    int vertex_count = vtx->getVertexCount();
	ImGui::InputInt("VertexCount", &vertex_count, 0, 0, ImGuiInputTextFlags_ReadOnly);

    VkIndexType index_type = vtx->getIndexType();
    std::string index_type_str = index_type == VK_INDEX_TYPE_UINT16 ? "UINT16"s : "UINT32"s;
    ImGui::InputText("IndexType", const_cast<char*>(index_type_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

    ImGui::SeparatorText("vkVertexBuffer");
    ImGui::PushID("vkVertexBuffer");
    printVulkanBufferImGUI(vtx->getVertexBuffer());
    ImGui::PopID();

    ImGui::SeparatorText("vkIndexBuffer");
    ImGui::PushID("vkIndexBuffer");
    printVulkanBufferImGUI(vtx->getIndexBuffer());
    ImGui::PopID();

    if(ImGui::TreeNode("VertexInputBindingDescription")) {
        VkPipelineVertexInputStateCreateInfo vtx_input_desc = vtx->getVertextInputInfo();
        size_t bind_sz = vtx_input_desc.vertexBindingDescriptionCount;
        for(size_t b = 0u; b < bind_sz; ++b) {
            ImGui::PushID(b);

            VkVertexInputBindingDescription bind_desc = vtx_input_desc.pVertexBindingDescriptions[b];

            ImGui::Text(("InputDescription"s + std::to_string(b)).c_str());

            int binding = bind_desc.binding;
            ImGui::InputInt("binding", &binding, 0, 0, ImGuiInputTextFlags_ReadOnly);

            int stride = bind_desc.stride;
            ImGui::InputInt("stride", &stride, 0, 0, ImGuiInputTextFlags_ReadOnly);

            std::string input_rate_str = bind_desc.inputRate == VK_VERTEX_INPUT_RATE_VERTEX ? "per vertex"s : "per instance"s;
            ImGui::InputText("inputRate", const_cast<char*>(input_rate_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

            ImGui::PopID();
        }

        ImGui::SeparatorText("VertexInputAttributeDescription");
        size_t attrib_sz = vtx_input_desc.vertexAttributeDescriptionCount;
        for(size_t a = 0u; a < attrib_sz; ++a) {
            ImGui::PushID(a + bind_sz);

            VkVertexInputAttributeDescription attr_desc = vtx_input_desc.pVertexAttributeDescriptions[a];

            ImGui::Text(("AttributeDescription"s + std::to_string(a)).c_str());

            int binding = attr_desc.binding;
            ImGui::InputInt("binding", &binding, 0, 0, ImGuiInputTextFlags_ReadOnly);

            int location = attr_desc.location;
            ImGui::InputInt("location", &location, 0, 0, ImGuiInputTextFlags_ReadOnly);

            std::string format_str = getFormatStr(attr_desc.format);
            ImGui::InputText("format", const_cast<char*>(format_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

            int offset = attr_desc.offset;
            ImGui::InputInt("offset", &offset, 0, 0, ImGuiInputTextFlags_ReadOnly);

            ImGui::PopID();
        }

        ImGui::TreePop();
    }
}

void printMeshNodeImGUI(std::shared_ptr<MeshNode> pMesh) {
    if(!pMesh) return;

    const MeshNode::MeshList& mesh_list = pMesh->GetMeshes();
    for(std::shared_ptr<ModelData> mode_data : mesh_list) {
        ImGui::PushID(mode_data->GetName().c_str());

        ImGui::SeparatorText(mode_data->GetName().c_str());

        std::shared_ptr<VertexBuffer> vtx = mode_data->GetVertexBuffer();

        if(vtx && ImGui::TreeNode("VertexBuffer")) {
            printVertexBufferImGUI(vtx, mode_data->GetPrimitiveTopology());

            ImGui::TreePop();
        }

        std::shared_ptr<Material> material = mode_data->GetMaterial();
        if(material && ImGui::TreeNode("Material")) {
            const MaterialProperties& mat_prop = material->GetMaterialProperties();

            if (ImGui::ColorEdit4("Diffuse", ((float*)&mat_prop.Diffuse) + 0)) {};
            if (ImGui::ColorEdit4("Specular", ((float*)&mat_prop.Specular) + 0)) {}
            if (ImGui::ColorEdit4("Emissive", ((float*)&mat_prop.Emissive) + 0)) {}
            if (ImGui::ColorEdit4("Ambient", ((float*)&mat_prop.Ambient) + 0)) {}
            if (ImGui::ColorEdit4("Reflectance", ((float*)&mat_prop.Reflectance) + 0)) {}
            if (ImGui::InputFloat("Opacity", const_cast<float*>(&mat_prop.Opacity), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
            if (ImGui::InputFloat("SpecularPower", const_cast<float*>(&mat_prop.SpecularPower), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
            if (ImGui::InputFloat("IndexOfRefraction", const_cast<float*>(&mat_prop.IndexOfRefraction), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
            if (ImGui::InputFloat("BumpIntensity", const_cast<float*>(&mat_prop.BumpIntensity), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
            if (ImGui::InputFloat("metallicFactor", const_cast<float*>(&mat_prop.metallicFactor), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
            if (ImGui::InputFloat("roughnessFactor", const_cast<float*>(&mat_prop.roughnessFactor), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
                                    
            std::string mat_textures_str = getMaterialTextures(mat_prop.HasTexture);
            ImGui::InputText("HasTextures", const_cast<char*>(mat_textures_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

            ImGui::TreePop();
        }

        if(ImGui::TreeNode("BoundingBox")) {
            ImGui::SeparatorText("BoundingBox");
            printBoundingBoxImGUI(mode_data->GetAABB());

            ImGui::SeparatorText("BoundingSphere");
            printBoundingSphereImGUI(mode_data->GetSphere());

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}

void printCameraNodeImGUI(std::shared_ptr<CameraNode> pCamera) {
    if(!pCamera) return;

    if (ImGui::TreeNode("ViewProjectionMatrix")) {
        printMatrixImGUI(pCamera->GetViewProjection());
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("ProjectionMatrix")) {
        printMatrixImGUI(pCamera->GetProjection());
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("ViewMatrix")) {
        printMatrixImGUI(pCamera->GetView());
        ImGui::TreePop();
    }

    std::shared_ptr<BasicCameraNode> pBasic_camera = std::dynamic_pointer_cast<BasicCameraNode>(pCamera);
    if(!pBasic_camera) return;

    printBoundingFrustumImGUI(pBasic_camera->GetFrustum());

	float cam_fov = pBasic_camera->GetFovYDeg();
	if (ImGui::InputFloat("FovY", const_cast<float*>(&cam_fov), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}

    float cam_near = pBasic_camera->GetNear();
    if (ImGui::InputFloat("Near", const_cast<float*>(&cam_near), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}

    float cam_far = pBasic_camera->GetFar();
    if (ImGui::InputFloat("Far", const_cast<float*>(&cam_far), 0.0F, 0.0F, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
}

void printAABBNodeImGUI(std::shared_ptr<AABBNode> pAABB) {
    if(!pAABB) return;

    printBoundingBoxImGUI(pAABB->getAABB());
}
