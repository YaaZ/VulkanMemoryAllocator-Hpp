#ifndef VULKAN_MEMORY_ALLOCATOR_VALIDATION_HPP
#define VULKAN_MEMORY_ALLOCATOR_VALIDATION_HPP

#define VMA_HPP_BINDING(FUNCTION) private:                                      \
friend struct ::VMA_HPP_NAMESPACE::detail::validation_traits::FUNCTION;         \
static_assert(!::VMA_HPP_NAMESPACE::detail::validation_traits::FUNCTION::value, \
#FUNCTION " is statically disabled!"); using VmaHppValidationTag = void; public:

#define VMA_HPP_GLOBAL_BINDING(FUNCTION) namespace detail { namespace validation { \
static_assert(!::VMA_HPP_NAMESPACE::detail::validation_traits::FUNCTION::value,    \
#FUNCTION " is statically disabled!"); using FUNCTION = std::true_type; } }

#define VMA_HPP_VALIDATE(...) namespace detail { namespace validation { \
  using namespace ::VMA_HPP_NAMESPACE::detail::validation_traits; \
  static_assert(HasBinding<vmaImportVulkanFunctionsFromVolk>::WithClasses<__VA_ARGS__>::value, "vmaImportVulkanFunctionsFromVolk binding is missing!"); \
  static_assert(HasBinding<vmaCreateAllocator>::WithClasses<__VA_ARGS__>::value, "vmaCreateAllocator binding is missing!"); \
  static_assert(HasBinding<vmaDestroyAllocator>::WithClasses<__VA_ARGS__>::value, "vmaDestroyAllocator binding is missing!"); \
  static_assert(HasBinding<vmaGetAllocatorInfo>::WithClasses<__VA_ARGS__>::value, "vmaGetAllocatorInfo binding is missing!"); \
  static_assert(HasBinding<vmaGetPhysicalDeviceProperties>::WithClasses<__VA_ARGS__>::value, "vmaGetPhysicalDeviceProperties binding is missing!"); \
  static_assert(HasBinding<vmaGetMemoryProperties>::WithClasses<__VA_ARGS__>::value, "vmaGetMemoryProperties binding is missing!"); \
  static_assert(HasBinding<vmaGetMemoryTypeProperties>::WithClasses<__VA_ARGS__>::value, "vmaGetMemoryTypeProperties binding is missing!"); \
  static_assert(HasBinding<vmaSetCurrentFrameIndex>::WithClasses<__VA_ARGS__>::value, "vmaSetCurrentFrameIndex binding is missing!"); \
  static_assert(HasBinding<vmaCalculateStatistics>::WithClasses<__VA_ARGS__>::value, "vmaCalculateStatistics binding is missing!"); \
  static_assert(HasBinding<vmaGetHeapBudgets>::WithClasses<__VA_ARGS__>::value, "vmaGetHeapBudgets binding is missing!"); \
  static_assert(HasBinding<vmaFindMemoryTypeIndex>::WithClasses<__VA_ARGS__>::value, "vmaFindMemoryTypeIndex binding is missing!"); \
  static_assert(HasBinding<vmaFindMemoryTypeIndexForBufferInfo>::WithClasses<__VA_ARGS__>::value, "vmaFindMemoryTypeIndexForBufferInfo binding is missing!"); \
  static_assert(HasBinding<vmaFindMemoryTypeIndexForImageInfo>::WithClasses<__VA_ARGS__>::value, "vmaFindMemoryTypeIndexForImageInfo binding is missing!"); \
  static_assert(HasBinding<vmaCreatePool>::WithClasses<__VA_ARGS__>::value, "vmaCreatePool binding is missing!"); \
  static_assert(HasBinding<vmaDestroyPool>::WithClasses<__VA_ARGS__>::value, "vmaDestroyPool binding is missing!"); \
  static_assert(HasBinding<vmaGetPoolStatistics>::WithClasses<__VA_ARGS__>::value, "vmaGetPoolStatistics binding is missing!"); \
  static_assert(HasBinding<vmaCalculatePoolStatistics>::WithClasses<__VA_ARGS__>::value, "vmaCalculatePoolStatistics binding is missing!"); \
  static_assert(HasBinding<vmaCheckPoolCorruption>::WithClasses<__VA_ARGS__>::value, "vmaCheckPoolCorruption binding is missing!"); \
  static_assert(HasBinding<vmaGetPoolName>::WithClasses<__VA_ARGS__>::value, "vmaGetPoolName binding is missing!"); \
  static_assert(HasBinding<vmaSetPoolName>::WithClasses<__VA_ARGS__>::value, "vmaSetPoolName binding is missing!"); \
  static_assert(HasBinding<vmaAllocateMemory>::WithClasses<__VA_ARGS__>::value, "vmaAllocateMemory binding is missing!"); \
  static_assert(HasBinding<vmaAllocateMemoryPages>::WithClasses<__VA_ARGS__>::value, "vmaAllocateMemoryPages binding is missing!"); \
  static_assert(HasBinding<vmaAllocateMemoryForBuffer>::WithClasses<__VA_ARGS__>::value, "vmaAllocateMemoryForBuffer binding is missing!"); \
  static_assert(HasBinding<vmaAllocateMemoryForImage>::WithClasses<__VA_ARGS__>::value, "vmaAllocateMemoryForImage binding is missing!"); \
  static_assert(HasBinding<vmaFreeMemory>::WithClasses<__VA_ARGS__>::value, "vmaFreeMemory binding is missing!"); \
  static_assert(HasBinding<vmaFreeMemoryPages>::WithClasses<__VA_ARGS__>::value, "vmaFreeMemoryPages binding is missing!"); \
  static_assert(HasBinding<vmaGetAllocationInfo>::WithClasses<__VA_ARGS__>::value, "vmaGetAllocationInfo binding is missing!"); \
  static_assert(HasBinding<vmaGetAllocationInfo2>::WithClasses<__VA_ARGS__>::value, "vmaGetAllocationInfo2 binding is missing!"); \
  static_assert(HasBinding<vmaSetAllocationUserData>::WithClasses<__VA_ARGS__>::value, "vmaSetAllocationUserData binding is missing!"); \
  static_assert(HasBinding<vmaSetAllocationName>::WithClasses<__VA_ARGS__>::value, "vmaSetAllocationName binding is missing!"); \
  static_assert(HasBinding<vmaGetAllocationMemoryProperties>::WithClasses<__VA_ARGS__>::value, "vmaGetAllocationMemoryProperties binding is missing!"); \
  static_assert(HasBinding<vmaGetMemoryWin32Handle>::WithClasses<__VA_ARGS__>::value, "vmaGetMemoryWin32Handle binding is missing!"); \
  static_assert(HasBinding<vmaMapMemory>::WithClasses<__VA_ARGS__>::value, "vmaMapMemory binding is missing!"); \
  static_assert(HasBinding<vmaUnmapMemory>::WithClasses<__VA_ARGS__>::value, "vmaUnmapMemory binding is missing!"); \
  static_assert(HasBinding<vmaFlushAllocation>::WithClasses<__VA_ARGS__>::value, "vmaFlushAllocation binding is missing!"); \
  static_assert(HasBinding<vmaInvalidateAllocation>::WithClasses<__VA_ARGS__>::value, "vmaInvalidateAllocation binding is missing!"); \
  static_assert(HasBinding<vmaFlushAllocations>::WithClasses<__VA_ARGS__>::value, "vmaFlushAllocations binding is missing!"); \
  static_assert(HasBinding<vmaInvalidateAllocations>::WithClasses<__VA_ARGS__>::value, "vmaInvalidateAllocations binding is missing!"); \
  static_assert(HasBinding<vmaCopyMemoryToAllocation>::WithClasses<__VA_ARGS__>::value, "vmaCopyMemoryToAllocation binding is missing!"); \
  static_assert(HasBinding<vmaCopyAllocationToMemory>::WithClasses<__VA_ARGS__>::value, "vmaCopyAllocationToMemory binding is missing!"); \
  static_assert(HasBinding<vmaCheckCorruption>::WithClasses<__VA_ARGS__>::value, "vmaCheckCorruption binding is missing!"); \
  static_assert(HasBinding<vmaBeginDefragmentation>::WithClasses<__VA_ARGS__>::value, "vmaBeginDefragmentation binding is missing!"); \
  static_assert(HasBinding<vmaEndDefragmentation>::WithClasses<__VA_ARGS__>::value, "vmaEndDefragmentation binding is missing!"); \
  static_assert(HasBinding<vmaBeginDefragmentationPass>::WithClasses<__VA_ARGS__>::value, "vmaBeginDefragmentationPass binding is missing!"); \
  static_assert(HasBinding<vmaEndDefragmentationPass>::WithClasses<__VA_ARGS__>::value, "vmaEndDefragmentationPass binding is missing!"); \
  static_assert(HasBinding<vmaBindBufferMemory>::WithClasses<__VA_ARGS__>::value, "vmaBindBufferMemory binding is missing!"); \
  static_assert(HasBinding<vmaBindBufferMemory2>::WithClasses<__VA_ARGS__>::value, "vmaBindBufferMemory2 binding is missing!"); \
  static_assert(HasBinding<vmaBindImageMemory>::WithClasses<__VA_ARGS__>::value, "vmaBindImageMemory binding is missing!"); \
  static_assert(HasBinding<vmaBindImageMemory2>::WithClasses<__VA_ARGS__>::value, "vmaBindImageMemory2 binding is missing!"); \
  static_assert(HasBinding<vmaCreateBuffer>::WithClasses<__VA_ARGS__>::value, "vmaCreateBuffer binding is missing!"); \
  static_assert(HasBinding<vmaCreateBufferWithAlignment>::WithClasses<__VA_ARGS__>::value, "vmaCreateBufferWithAlignment binding is missing!"); \
  static_assert(HasBinding<vmaCreateAliasingBuffer>::WithClasses<__VA_ARGS__>::value, "vmaCreateAliasingBuffer binding is missing!"); \
  static_assert(HasBinding<vmaCreateAliasingBuffer2>::WithClasses<__VA_ARGS__>::value, "vmaCreateAliasingBuffer2 binding is missing!"); \
  static_assert(HasBinding<vmaDestroyBuffer>::WithClasses<__VA_ARGS__>::value, "vmaDestroyBuffer binding is missing!"); \
  static_assert(HasBinding<vmaCreateImage>::WithClasses<__VA_ARGS__>::value, "vmaCreateImage binding is missing!"); \
  static_assert(HasBinding<vmaCreateAliasingImage>::WithClasses<__VA_ARGS__>::value, "vmaCreateAliasingImage binding is missing!"); \
  static_assert(HasBinding<vmaCreateAliasingImage2>::WithClasses<__VA_ARGS__>::value, "vmaCreateAliasingImage2 binding is missing!"); \
  static_assert(HasBinding<vmaDestroyImage>::WithClasses<__VA_ARGS__>::value, "vmaDestroyImage binding is missing!"); \
  static_assert(HasBinding<vmaCreateVirtualBlock>::WithClasses<__VA_ARGS__>::value, "vmaCreateVirtualBlock binding is missing!"); \
  static_assert(HasBinding<vmaDestroyVirtualBlock>::WithClasses<__VA_ARGS__>::value, "vmaDestroyVirtualBlock binding is missing!"); \
  static_assert(HasBinding<vmaIsVirtualBlockEmpty>::WithClasses<__VA_ARGS__>::value, "vmaIsVirtualBlockEmpty binding is missing!"); \
  static_assert(HasBinding<vmaGetVirtualAllocationInfo>::WithClasses<__VA_ARGS__>::value, "vmaGetVirtualAllocationInfo binding is missing!"); \
  static_assert(HasBinding<vmaVirtualAllocate>::WithClasses<__VA_ARGS__>::value, "vmaVirtualAllocate binding is missing!"); \
  static_assert(HasBinding<vmaVirtualFree>::WithClasses<__VA_ARGS__>::value, "vmaVirtualFree binding is missing!"); \
  static_assert(HasBinding<vmaClearVirtualBlock>::WithClasses<__VA_ARGS__>::value, "vmaClearVirtualBlock binding is missing!"); \
  static_assert(HasBinding<vmaSetVirtualAllocationUserData>::WithClasses<__VA_ARGS__>::value, "vmaSetVirtualAllocationUserData binding is missing!"); \
  static_assert(HasBinding<vmaGetVirtualBlockStatistics>::WithClasses<__VA_ARGS__>::value, "vmaGetVirtualBlockStatistics binding is missing!"); \
  static_assert(HasBinding<vmaCalculateVirtualBlockStatistics>::WithClasses<__VA_ARGS__>::value, "vmaCalculateVirtualBlockStatistics binding is missing!"); \
  static_assert(HasBinding<vmaBuildVirtualBlockStatsString>::WithClasses<__VA_ARGS__>::value, "vmaBuildVirtualBlockStatsString binding is missing!"); \
  static_assert(HasBinding<vmaFreeVirtualBlockStatsString>::WithClasses<__VA_ARGS__>::value, "vmaFreeVirtualBlockStatsString binding is missing!"); \
  static_assert(HasBinding<vmaBuildStatsString>::WithClasses<__VA_ARGS__>::value, "vmaBuildStatsString binding is missing!"); \
  static_assert(HasBinding<vmaFreeStatsString>::WithClasses<__VA_ARGS__>::value, "vmaFreeStatsString binding is missing!"); \
} } static_assert(true, "")

namespace VMA_HPP_NAMESPACE {
  namespace detail {
    namespace validation_traits {
      template<class, class, class = void> struct BindingInClass : std::false_type {};

      template<class Function> struct HasBinding {
        template<class...> struct WithClasses : Function {};
      };
      template<class Function> template<class Class, class... Remaining>
      struct HasBinding<Function>::WithClasses<Class, Remaining...> : std::integral_constant<bool,
        BindingInClass<Class, Function>::value || WithClasses<Remaining...>::value> {};
      template<> template<class... Classes>
      struct HasBinding<std::true_type>::WithClasses<Classes...> : std::true_type {};

      template<bool value = true> using Default = std::integral_constant<bool, value>;

      struct vmaImportVulkanFunctionsFromVolk : Default<
#ifdef VOLK_HEADER_VERSION
false
#endif
> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaImportVulkanFunctionsFromVolk, vmaImportVulkanFunctionsFromVolk::InClass<T>> : std::true_type {};
      struct vmaCreateAllocator : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateAllocator, vmaCreateAllocator::InClass<T>> : std::true_type {};
      struct vmaDestroyAllocator : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaDestroyAllocator, vmaDestroyAllocator::InClass<T>> : std::true_type {};
      struct vmaGetAllocatorInfo : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetAllocatorInfo, vmaGetAllocatorInfo::InClass<T>> : std::true_type {};
      struct vmaGetPhysicalDeviceProperties : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetPhysicalDeviceProperties, vmaGetPhysicalDeviceProperties::InClass<T>> : std::true_type {};
      struct vmaGetMemoryProperties : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetMemoryProperties, vmaGetMemoryProperties::InClass<T>> : std::true_type {};
      struct vmaGetMemoryTypeProperties : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetMemoryTypeProperties, vmaGetMemoryTypeProperties::InClass<T>> : std::true_type {};
      struct vmaSetCurrentFrameIndex : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaSetCurrentFrameIndex, vmaSetCurrentFrameIndex::InClass<T>> : std::true_type {};
      struct vmaCalculateStatistics : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCalculateStatistics, vmaCalculateStatistics::InClass<T>> : std::true_type {};
      struct vmaGetHeapBudgets : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetHeapBudgets, vmaGetHeapBudgets::InClass<T>> : std::true_type {};
      struct vmaFindMemoryTypeIndex : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFindMemoryTypeIndex, vmaFindMemoryTypeIndex::InClass<T>> : std::true_type {};
      struct vmaFindMemoryTypeIndexForBufferInfo : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFindMemoryTypeIndexForBufferInfo, vmaFindMemoryTypeIndexForBufferInfo::InClass<T>> : std::true_type {};
      struct vmaFindMemoryTypeIndexForImageInfo : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFindMemoryTypeIndexForImageInfo, vmaFindMemoryTypeIndexForImageInfo::InClass<T>> : std::true_type {};
      struct vmaCreatePool : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreatePool, vmaCreatePool::InClass<T>> : std::true_type {};
      struct vmaDestroyPool : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaDestroyPool, vmaDestroyPool::InClass<T>> : std::true_type {};
      struct vmaGetPoolStatistics : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetPoolStatistics, vmaGetPoolStatistics::InClass<T>> : std::true_type {};
      struct vmaCalculatePoolStatistics : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCalculatePoolStatistics, vmaCalculatePoolStatistics::InClass<T>> : std::true_type {};
      struct vmaCheckPoolCorruption : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCheckPoolCorruption, vmaCheckPoolCorruption::InClass<T>> : std::true_type {};
      struct vmaGetPoolName : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetPoolName, vmaGetPoolName::InClass<T>> : std::true_type {};
      struct vmaSetPoolName : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaSetPoolName, vmaSetPoolName::InClass<T>> : std::true_type {};
      struct vmaAllocateMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaAllocateMemory, vmaAllocateMemory::InClass<T>> : std::true_type {};
      struct vmaAllocateMemoryPages : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaAllocateMemoryPages, vmaAllocateMemoryPages::InClass<T>> : std::true_type {};
      struct vmaAllocateMemoryForBuffer : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaAllocateMemoryForBuffer, vmaAllocateMemoryForBuffer::InClass<T>> : std::true_type {};
      struct vmaAllocateMemoryForImage : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaAllocateMemoryForImage, vmaAllocateMemoryForImage::InClass<T>> : std::true_type {};
      struct vmaFreeMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFreeMemory, vmaFreeMemory::InClass<T>> : std::true_type {};
      struct vmaFreeMemoryPages : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFreeMemoryPages, vmaFreeMemoryPages::InClass<T>> : std::true_type {};
      struct vmaGetAllocationInfo : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetAllocationInfo, vmaGetAllocationInfo::InClass<T>> : std::true_type {};
      struct vmaGetAllocationInfo2 : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetAllocationInfo2, vmaGetAllocationInfo2::InClass<T>> : std::true_type {};
      struct vmaSetAllocationUserData : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaSetAllocationUserData, vmaSetAllocationUserData::InClass<T>> : std::true_type {};
      struct vmaSetAllocationName : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaSetAllocationName, vmaSetAllocationName::InClass<T>> : std::true_type {};
      struct vmaGetAllocationMemoryProperties : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetAllocationMemoryProperties, vmaGetAllocationMemoryProperties::InClass<T>> : std::true_type {};
      struct vmaGetMemoryWin32Handle : Default<
#if VMA_EXTERNAL_MEMORY_WIN32
false
#endif
> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetMemoryWin32Handle, vmaGetMemoryWin32Handle::InClass<T>> : std::true_type {};
      struct vmaMapMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaMapMemory, vmaMapMemory::InClass<T>> : std::true_type {};
      struct vmaUnmapMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaUnmapMemory, vmaUnmapMemory::InClass<T>> : std::true_type {};
      struct vmaFlushAllocation : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFlushAllocation, vmaFlushAllocation::InClass<T>> : std::true_type {};
      struct vmaInvalidateAllocation : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaInvalidateAllocation, vmaInvalidateAllocation::InClass<T>> : std::true_type {};
      struct vmaFlushAllocations : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFlushAllocations, vmaFlushAllocations::InClass<T>> : std::true_type {};
      struct vmaInvalidateAllocations : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaInvalidateAllocations, vmaInvalidateAllocations::InClass<T>> : std::true_type {};
      struct vmaCopyMemoryToAllocation : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCopyMemoryToAllocation, vmaCopyMemoryToAllocation::InClass<T>> : std::true_type {};
      struct vmaCopyAllocationToMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCopyAllocationToMemory, vmaCopyAllocationToMemory::InClass<T>> : std::true_type {};
      struct vmaCheckCorruption : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCheckCorruption, vmaCheckCorruption::InClass<T>> : std::true_type {};
      struct vmaBeginDefragmentation : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBeginDefragmentation, vmaBeginDefragmentation::InClass<T>> : std::true_type {};
      struct vmaEndDefragmentation : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaEndDefragmentation, vmaEndDefragmentation::InClass<T>> : std::true_type {};
      struct vmaBeginDefragmentationPass : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBeginDefragmentationPass, vmaBeginDefragmentationPass::InClass<T>> : std::true_type {};
      struct vmaEndDefragmentationPass : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaEndDefragmentationPass, vmaEndDefragmentationPass::InClass<T>> : std::true_type {};
      struct vmaBindBufferMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBindBufferMemory, vmaBindBufferMemory::InClass<T>> : std::true_type {};
      struct vmaBindBufferMemory2 : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBindBufferMemory2, vmaBindBufferMemory2::InClass<T>> : std::true_type {};
      struct vmaBindImageMemory : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBindImageMemory, vmaBindImageMemory::InClass<T>> : std::true_type {};
      struct vmaBindImageMemory2 : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBindImageMemory2, vmaBindImageMemory2::InClass<T>> : std::true_type {};
      struct vmaCreateBuffer : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateBuffer, vmaCreateBuffer::InClass<T>> : std::true_type {};
      struct vmaCreateBufferWithAlignment : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateBufferWithAlignment, vmaCreateBufferWithAlignment::InClass<T>> : std::true_type {};
      struct vmaCreateAliasingBuffer : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateAliasingBuffer, vmaCreateAliasingBuffer::InClass<T>> : std::true_type {};
      struct vmaCreateAliasingBuffer2 : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateAliasingBuffer2, vmaCreateAliasingBuffer2::InClass<T>> : std::true_type {};
      struct vmaDestroyBuffer : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaDestroyBuffer, vmaDestroyBuffer::InClass<T>> : std::true_type {};
      struct vmaCreateImage : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateImage, vmaCreateImage::InClass<T>> : std::true_type {};
      struct vmaCreateAliasingImage : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateAliasingImage, vmaCreateAliasingImage::InClass<T>> : std::true_type {};
      struct vmaCreateAliasingImage2 : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateAliasingImage2, vmaCreateAliasingImage2::InClass<T>> : std::true_type {};
      struct vmaDestroyImage : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaDestroyImage, vmaDestroyImage::InClass<T>> : std::true_type {};
      struct vmaCreateVirtualBlock : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCreateVirtualBlock, vmaCreateVirtualBlock::InClass<T>> : std::true_type {};
      struct vmaDestroyVirtualBlock : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaDestroyVirtualBlock, vmaDestroyVirtualBlock::InClass<T>> : std::true_type {};
      struct vmaIsVirtualBlockEmpty : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaIsVirtualBlockEmpty, vmaIsVirtualBlockEmpty::InClass<T>> : std::true_type {};
      struct vmaGetVirtualAllocationInfo : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetVirtualAllocationInfo, vmaGetVirtualAllocationInfo::InClass<T>> : std::true_type {};
      struct vmaVirtualAllocate : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaVirtualAllocate, vmaVirtualAllocate::InClass<T>> : std::true_type {};
      struct vmaVirtualFree : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaVirtualFree, vmaVirtualFree::InClass<T>> : std::true_type {};
      struct vmaClearVirtualBlock : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaClearVirtualBlock, vmaClearVirtualBlock::InClass<T>> : std::true_type {};
      struct vmaSetVirtualAllocationUserData : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaSetVirtualAllocationUserData, vmaSetVirtualAllocationUserData::InClass<T>> : std::true_type {};
      struct vmaGetVirtualBlockStatistics : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaGetVirtualBlockStatistics, vmaGetVirtualBlockStatistics::InClass<T>> : std::true_type {};
      struct vmaCalculateVirtualBlockStatistics : Default<false> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaCalculateVirtualBlockStatistics, vmaCalculateVirtualBlockStatistics::InClass<T>> : std::true_type {};
      struct vmaBuildVirtualBlockStatsString : Default<
#if VMA_STATS_STRING_ENABLED
false
#endif
> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBuildVirtualBlockStatsString, vmaBuildVirtualBlockStatsString::InClass<T>> : std::true_type {};
      struct vmaFreeVirtualBlockStatsString : Default<
#if VMA_STATS_STRING_ENABLED
false
#endif
> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFreeVirtualBlockStatsString, vmaFreeVirtualBlockStatsString::InClass<T>> : std::true_type {};
      struct vmaBuildStatsString : Default<
#if VMA_STATS_STRING_ENABLED
false
#endif
> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaBuildStatsString, vmaBuildStatsString::InClass<T>> : std::true_type {};
      struct vmaFreeStatsString : Default<
#if VMA_STATS_STRING_ENABLED
false
#endif
> { template<class T> using InClass = T::VmaHppValidationTag; };
      template<class T> struct BindingInClass<T, vmaFreeStatsString, vmaFreeStatsString::InClass<T>> : std::true_type {};
    }
  }
}
#endif
