#include "node_menu_ui.h"

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../../scene/scene.h"
#include "../views/human_view.h"
#include "../../scene/nodes/scene_node.h"
#include "../../scene/nodes/mesh_node.h"
#include "../../scene/nodes/camera_node.h"
#include "../../scene/nodes/aabb_node.h"

NodeMenuUI::NodeMenuUI() {
}

NodeMenuUI::~NodeMenuUI() {}

bool NodeMenuUI::VOnRestore() {
    return true;
}

void DrawNodes(const std::shared_ptr<SceneNode>& current_node) {

}

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
    using namespace std;
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
    using namespace std;
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
    using namespace std;
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

bool NodeMenuUI::VOnRender(const GameTimerDelta& delta) {
    using namespace std::literals;

    Application& app = Application::Get();
    std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
    std::shared_ptr<HumanView> human_view = game_logic->GetHumanView();
    if(!human_view) return true;
    std::shared_ptr<Scene> scene = human_view->VGetScene();
    if(!scene) return true;

    if (ImGui::Begin("Nodes Menu")) {
        if (ImGui::CollapsingHeader("Flat Hierarchy")) {
            const std::vector<Scene::Hierarchy>& hierarchy = scene->getHierarchy();
            size_t node_ct = hierarchy.size();
            for(Scene::NodeIndex node_index = 0u; node_index < node_ct; ++node_index) {
                const Scene::Hierarchy& hierarchy_node = scene->getNodeHierarchy(node_index);

                Scene::NodeTypeFlags node_type_flags = scene->getNodeTypeFlags(node_index);
                std::string node_type_flags_str = "f"s;
                if(!node_type_flags) node_type_flags_str += "/N"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_MESH) node_type_flags_str += "/M"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_LIGHT) node_type_flags_str += "/L"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_CAMERA) node_type_flags_str += "/C"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_SHADOW_CAMERA) node_type_flags_str += "/Sh"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_AABB) node_type_flags_str += "/AABB"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_SPHERE) node_type_flags_str += "/Sp"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_BONE) node_type_flags_str += "/B"s;

                const std::unordered_map<Scene::NodeIndex, Scene::NameIndex>& node_name_map = scene->getNodeNameMap();
                const std::vector<std::string>& node_names = scene->getNodeNames();
                std::string node_name = node_name_map.contains(node_index) ? "--> n-"s + node_names.at(node_name_map.at(node_index)) : "-- > n-N"s;

                std::string header = "i"s + std::to_string(node_index)
                                   + " p"s + (hierarchy_node.parent != Scene::NO_INDEX ? std::to_string(hierarchy_node.parent) : "N"s)
                                   + " c"s + (hierarchy_node.first_child != Scene::NO_INDEX ? std::to_string(hierarchy_node.first_child) : "N"s)
                                   + " s"s + (hierarchy_node.next_sibling != Scene::NO_INDEX ? std::to_string(hierarchy_node.next_sibling) : "N"s)
                                   + " l"s + std::to_string(hierarchy_node.level)
                                   + " "s + node_type_flags_str
                                   + " "s + node_name;
                if (ImGui::TreeNode(header.c_str())) {
                    if (ImGui::TreeNode("Hierarchy")) {
						if (ImGui::BeginTable("Hierarchy Table", 3)) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);

                            int parent = hierarchy_node.parent;
		                    ImGui::InputInt("Parent", &parent, 0, 0, ImGuiInputTextFlags_ReadOnly);

                            int first_child = hierarchy_node.first_child;
                            ImGui::InputInt("Child", &first_child, 0, 0, ImGuiInputTextFlags_ReadOnly);

                            int next_sibling = hierarchy_node.next_sibling;
                            ImGui::InputInt("Sibling", &next_sibling, 0, 0, ImGuiInputTextFlags_ReadOnly);

                            int level = hierarchy_node.level;
                            ImGui::InputInt("Level", &level, 0, 0, ImGuiInputTextFlags_ReadOnly);
                            
                            ImGui::EndTable();
                        }
                        
						ImGui::TreePop();
					}
                    
                    if (ImGui::TreeNode("Properties")) {
                        
                        ImGui::SeparatorText("Properties");

                        if(node_name_map.contains(node_index)) {
                            std::string node_name = node_names.at(node_name_map.at(node_index)).c_str();
                            ImGui::InputText("Name", const_cast<char*>(node_name.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
                        }

                        if(scene->getNodeTypeFlagsMap().contains(node_index)) {
                            std::string node_type_flags_raw = std::to_string(node_type_flags);
                            std::string node_type_v = node_type_flags_raw + " --> " + node_type_flags_str;
                            ImGui::InputText("Type Flags", const_cast<char*>(node_type_v.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
                        }

                        ImGui::SeparatorText("Transforms");

                        if (ImGui::TreeNode("Local Transform")) {
        					if (ImGui::InputFloat4("R1", ((float*)&scene->getNodeLocalTransform(node_index)) + 0, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R2", ((float*)&scene->getNodeLocalTransform(node_index)) + 4, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R3", ((float*)&scene->getNodeLocalTransform(node_index)) + 8, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R4", ((float*)&scene->getNodeLocalTransform(node_index)) + 12, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					ImGui::TreePop();
        				}

                        if (ImGui::TreeNode("Global Transform")) {
        					if (ImGui::InputFloat4("R1", ((float*)&scene->getNodeGlobalTransform(node_index)) + 0, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R2", ((float*)&scene->getNodeGlobalTransform(node_index)) + 4, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R3", ((float*)&scene->getNodeGlobalTransform(node_index)) + 8, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R4", ((float*)&scene->getNodeGlobalTransform(node_index)) + 12, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					ImGui::TreePop();
        				}

                        if (ImGui::TreeNode("Decompose Local")) {
					    	glm::vec3 scale_xm;
					    	glm::quat rotation_xm;
					    	glm::vec3 translation_xm;

                            glm::mat4x4 mat = scene->getNodeLocalTransform(node_index);
                            translation_xm = mat[3];

                            for (int i = 0; i < 3; ++i) {
                                scale_xm[i] = glm::length(mat[i]);
                                mat[i] /= scale_xm[i];
                            }
                        
                            rotation_xm = glm::toQuat(mat);

					    	glm::vec3 pyr_xm = glm::eulerAngles(rotation_xm);
					    	glm::vec3 ypr_xm(
					    		glm::degrees(pyr_xm.y),
					    		glm::degrees(pyr_xm.x),
					    		glm::degrees(pyr_xm.z)
					    	);

					    	if (ImGui::InputFloat4("Rq", ((float*)&rotation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Sc", ((float*)&scale_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Tr", ((float*)&translation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Ypr", ((float*)&ypr_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	
					    	ImGui::TreePop();
					    }

                        if (ImGui::TreeNode("Decompose Global")) {
					    	glm::vec3 scale_xm;
					    	glm::quat rotation_xm;
					    	glm::vec3 translation_xm;

                            glm::mat4x4 mat = scene->getNodeGlobalTransform(node_index);
                            translation_xm = mat[3];

                            for (int i = 0; i < 3; ++i) {
                                scale_xm[i] = glm::length(mat[i]);
                                mat[i] /= scale_xm[i];
                            }
                        
                            rotation_xm = glm::toQuat(mat);

					    	glm::vec3 pyr_xm = glm::eulerAngles(rotation_xm);
					    	glm::vec3 ypr_xm(
					    		glm::degrees(pyr_xm.y),
					    		glm::degrees(pyr_xm.x),
					    		glm::degrees(pyr_xm.z)
					    	);

					    	if (ImGui::InputFloat4("Rq", ((float*)&rotation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Sc", ((float*)&scale_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Tr", ((float*)&translation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Ypr", ((float*)&ypr_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	
					    	ImGui::TreePop();
					    }

                        ImGui::SeparatorText("Node Types");

                        std::shared_ptr<SceneNode> pMeshNode = scene->getProperty(node_index, Scene::NODE_TYPE_FLAG_MESH);
                        if(pMeshNode && ImGui::TreeNode("Mesh")) {
                            std::shared_ptr<MeshNode> pMesh = std::dynamic_pointer_cast<MeshNode>(pMeshNode);
                            const MeshNode::MeshList& mesh_list = pMesh->GetMeshes();
                            for(std::shared_ptr<ModelData> mode_data : mesh_list) {
                                ImGui::PushID(mode_data->GetName().c_str());

                                ImGui::SeparatorText(mode_data->GetName().c_str());

                                std::shared_ptr<VertexBuffer> vtx = mode_data->GetVertexBuffer();

                                if(vtx && ImGui::TreeNode("VertexBuffer")) {
                                    std::string topology_str = getPrimitiveTopologyStr(mode_data->GetPrimitiveTopology());
                                    ImGui::InputText("PrimitiveTopology", const_cast<char*>(topology_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

                                    int index_count = mode_data->GetIndexCount();
		                            ImGui::InputInt("IndexCount", &index_count, 0, 0, ImGuiInputTextFlags_ReadOnly);

                                    int vertex_count = mode_data->GetVertexCount();
		                            ImGui::InputInt("VertexCount", &vertex_count, 0, 0, ImGuiInputTextFlags_ReadOnly);

                                    VkIndexType index_type = vtx->getIndexType();
                                    std::string index_type_str = index_type == VK_INDEX_TYPE_UINT16 ? "UINT16"s : "UINT32"s;
                                    ImGui::InputText("IndexType", const_cast<char*>(index_type_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

                                    std::shared_ptr<VulkanBuffer> vk_vtx_buffer = vtx->getVertexBuffer();

                                    int vtx_buff_sz = vk_vtx_buffer->getSize();
                                    ImGui::InputInt("VertexBufferSize", &vtx_buff_sz, 0, 0, ImGuiInputTextFlags_ReadOnly);

                                    std::string vtx_mem_prop_str = getMemoryPropertyStr(vk_vtx_buffer->getProperties());
                                    ImGui::InputText("VertexBufferMemoryProperty", const_cast<char*>(vtx_mem_prop_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

                                    std::string vtx_usage_str = getBufferUsageStr(vk_vtx_buffer->getUsage());
                                    ImGui::InputText("VertexBufferUsage", const_cast<char*>(vtx_usage_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

                                    std::shared_ptr<VulkanBuffer> vk_idx_buffer = vtx->getIndexBuffer();

                                    int idx_buff_sz = vk_idx_buffer->getSize();
                                    ImGui::InputInt("IndexBufferSize", &idx_buff_sz, 0, 0, ImGuiInputTextFlags_ReadOnly);

                                    std::string idx_mem_prop_str = getMemoryPropertyStr(vk_idx_buffer->getProperties());
                                    ImGui::InputText("IndexBufferMemoryProperty", const_cast<char*>(idx_mem_prop_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);

                                    std::string idx_usage_str = getBufferUsageStr(vk_idx_buffer->getUsage());
                                    ImGui::InputText("IndexBufferUsage", const_cast<char*>(idx_usage_str.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
                                
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

                                ImGui::PopID();
                            }

                            ImGui::TreePop();
                        }

                        std::shared_ptr<SceneNode> pCameraNode = scene->getProperty(node_index, Scene::NODE_TYPE_FLAG_CAMERA);
                        if(pCameraNode) {
                            std::shared_ptr<CameraNode> pCamera = std::dynamic_pointer_cast<CameraNode>(pCameraNode);
                        }

                        std::shared_ptr<SceneNode> pAABBNode = scene->getProperty(node_index, Scene::NODE_TYPE_FLAG_AABB);
                        if(pAABBNode) {
                            std::shared_ptr<AABBNode> pAABB = std::dynamic_pointer_cast<AABBNode>(pAABBNode);
                        }
                        
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
            }
        }
		//DrawNodes(m_scene->GetRootCast());
	}
	ImGui::End();

    return true;
}

void NodeMenuUI::VOnUpdate(const GameTimerDelta& delta) {}

int NodeMenuUI::VGetZOrder() const {
    return 1;
}

void NodeMenuUI::VSetZOrder(int const zOrder) {}