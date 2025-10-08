#ifndef VULKAN_MEMORY_ALLOCATOR_RAII_HPP
#define VULKAN_MEMORY_ALLOCATOR_RAII_HPP

#include "vk_mem_alloc.hpp"

#if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
namespace VMA_HPP_NAMESPACE {
  namespace VMA_HPP_RAII_NAMESPACE {

    class Pool;
    class Allocation;
    class DefragmentationContext;
    class Allocator;
    class VirtualAllocation;
    class VirtualBlock;
    class Buffer;
    class Image;

    VMA_HPP_GLOBAL_BINDING(vmaCreateAllocator)
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type
    createAllocator(VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Instance const & instance,
                    VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const & device,
                    AllocatorCreateInfo createInfo);

    VMA_HPP_GLOBAL_BINDING(vmaCreateVirtualBlock)
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type
    createVirtualBlock(const VirtualBlockCreateInfo& createInfo);

#ifdef VOLK_HEADER_VERSION
    VMA_HPP_GLOBAL_BINDING(vmaImportVulkanFunctionsFromVolk) // Ignored - no RAII variant
#endif

    namespace detail {

      class util {

        static bool isEmpty(const vk::raii::Buffer& t) { return !*t; }
        static bool isEmpty(const vk::raii::Image& t) { return !*t; }
        template<class T> static bool isEmpty(const T& t) { return !t; }
        template<class A, class B> static bool isEmpty(const std::pair<A, B>& t) { return isEmpty(t.second); }

      public:
#ifdef VULKAN_HPP_NO_EXCEPTIONS
        template<class Return, class Create = Return, class Before, class T, class... After> static VULKAN_HPP_NAMESPACE::ResultValue<Return>
        wrap(Before&& before, VULKAN_HPP_NAMESPACE::ResultValue<T> t, After&&... after) VULKAN_HPP_NOEXCEPT {
          if (isEmpty(t.value)) return VULKAN_HPP_NAMESPACE::ResultValue<Return>(t.result, static_cast<Return>(Create(nullptr)));
          return VULKAN_HPP_NAMESPACE::ResultValue<Return>(t.result, static_cast<Return>(Create(std::forward<Before>(before), std::move(t.value), std::forward<After>(after)...)));
        }
#else
        template<class Return, class Create = Return, class Before, class T, class... After> static Return
        wrap(Before&& before, T t, After&&... after) {
          if (isEmpty(t)) return static_cast<Return>(Create(nullptr));
          return static_cast<Return>(Create(std::forward<Before>(before), std::move(t), std::forward<After>(after)...));
        }
#endif
      };

      template<class> using CTypeVoid = void;
      template<class T, class=void> struct CType { using type = T; };
      template<class T> struct CType<T, CTypeVoid<typename T::CType>> { using type = T::CType; };

      template<class T, std::size_t Main, class... Components> class RAII : protected std::tuple<Components...> {
        using Tuple = std::tuple<Components...>;
      public:
        using CppType = std::tuple_element<Main, Tuple>::type;
        using CType   = CType<CppType>::type;

        RAII(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}

        explicit RAII(Components... components) VULKAN_HPP_NOEXCEPT : Tuple(components...) {}

        ~RAII() {
          VULKAN_HPP_STATIC_ASSERT(std::is_base_of<RAII, T>::value, "CRTP violation!");
          VULKAN_HPP_STATIC_ASSERT(sizeof(T) == sizeof(Tuple), "Class and underlying tuple sizes do not match!");
          ((T*) this)->clear();
        }

        RAII() VULKAN_HPP_NOEXCEPT = delete;
        RAII(RAII const&) VULKAN_HPP_NOEXCEPT = delete;
        RAII& operator=(RAII const &) VULKAN_HPP_NOEXCEPT = delete;

        RAII(RAII && rhs) VULKAN_HPP_NOEXCEPT : Tuple(std::exchange((Tuple&) rhs, Tuple())) {}
        RAII& operator=(RAII && rhs) VULKAN_HPP_NOEXCEPT {
          if (this != &rhs) std::swap((Tuple&) *this, (Tuple&) rhs);
          return *this;
        }

        CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return std::get<Main>(*this); }
        CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(std::get<Main>(*this)); }
        operator CppType() const VULKAN_HPP_NOEXCEPT { return std::get<Main>(*this); }

        CppType release() VULKAN_HPP_NOEXCEPT {
          CppType result = *this;
          (Tuple&) *this = Tuple();
          return result;
        }

        void swap(T & rhs) VULKAN_HPP_NOEXCEPT { std::swap((Tuple&) *this, (Tuple&) rhs); }
      };
    }

#if VMA_STATS_STRING_ENABLED
    class StatsString : public detail::RAII<StatsString, 1, void*, char*, void (*)(void*, char*)> {
      StatsString(void* owner, char* string, void (*destructor)(void*, char*)) VULKAN_HPP_NOEXCEPT : RAII(owner, string, destructor) {}
    public:
      using RAII::RAII;

      template<class Owner, void (*destructor)(typename Owner::CType, char*)>
      static StatsString wrap(Owner const & owner, char * string) VULKAN_HPP_NOEXCEPT {
        return StatsString((void*) owner, string, [](void* owner, char* string) { destructor((typename Owner::CType) owner, string); });
      }

      void clear() VULKAN_HPP_NOEXCEPT {
        if ((char*) *this) {
          std::get<2>(*this)(std::get<0>(*this), (char*) *this);
          release();
        }
      }
    };
#endif

    class Pool : public detail::RAII<Pool, 1,
        VMA_HPP_NAMESPACE::Allocator,
        VMA_HPP_NAMESPACE::Pool> {
    public:
      using RAII::RAII;
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaCreatePool)
      Pool(const VMA_HPP_NAMESPACE::Allocator& allocator, const PoolCreateInfo& createInfo) :
        RAII(allocator, allocator.createPool(createInfo)) {}
#endif

      VMA_HPP_NAMESPACE::Allocator getAllocator() const VULKAN_HPP_NOEXCEPT { return std::get<0>(*this); }

      VMA_HPP_BINDING(vmaDestroyPool)
      void
      clear() VULKAN_HPP_NOEXCEPT {
        if (**this) {
          getAllocator().destroyPool(*this);
          release();
        }
      }

      VMA_HPP_BINDING(vmaGetPoolStatistics)
      VULKAN_HPP_NODISCARD Statistics
      getStatistics() const VULKAN_HPP_NOEXCEPT {
        return getAllocator().getPoolStatistics(*this);
      }

      VMA_HPP_BINDING(vmaCalculatePoolStatistics)
      VULKAN_HPP_NODISCARD DetailedStatistics
      calculateStatistics() const VULKAN_HPP_NOEXCEPT {
        return getAllocator().calculatePoolStatistics(*this);
      }

      VMA_HPP_BINDING(vmaCheckPoolCorruption)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      checkCorruption() const {
        return getAllocator().checkPoolCorruption(*this);
      }

      VMA_HPP_BINDING(vmaGetPoolName)
      VULKAN_HPP_NODISCARD const char*
      getName() const VULKAN_HPP_NOEXCEPT {
        return getAllocator().getPoolName(*this);
      }

      VMA_HPP_BINDING(vmaSetPoolName)
      void
      setPoolName(const char* name) const VULKAN_HPP_NOEXCEPT {
        getAllocator().setPoolName(*this, name);
      }
    };

    class Allocation : public detail::RAII<Allocation, 1,
        VMA_HPP_NAMESPACE::Allocator,
        VMA_HPP_NAMESPACE::Allocation> {
    public:
      using RAII::RAII;
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaAllocateMemory)
      Allocation(const VMA_HPP_NAMESPACE::Allocator& allocator,
                 const VULKAN_HPP_NAMESPACE::MemoryRequirements& memoryRequirements,
                 const AllocationCreateInfo& createInfo,
                 VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) :
        RAII(allocator, allocator.allocateMemory(memoryRequirements, createInfo, allocationInfo)) {}

      VMA_HPP_BINDING(vmaAllocateMemoryForBuffer)
      Allocation(const VMA_HPP_NAMESPACE::Allocator& allocator,
                 VULKAN_HPP_NAMESPACE::Buffer buffer,
                 const AllocationCreateInfo& createInfo,
                 VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) :
        RAII(allocator, allocator.allocateMemoryForBuffer(buffer, createInfo, allocationInfo)) {}

      VMA_HPP_BINDING(vmaAllocateMemoryForImage)
      Allocation(const VMA_HPP_NAMESPACE::Allocator& allocator,
                 VULKAN_HPP_NAMESPACE::Image image,
                 const AllocationCreateInfo& createInfo,
                 VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) :
        RAII(allocator, allocator.allocateMemoryForImage(image, createInfo, allocationInfo)) {}
#endif
      
      VMA_HPP_NAMESPACE::Allocator getAllocator() const VULKAN_HPP_NOEXCEPT { return std::get<0>(*this); }
      
      VMA_HPP_BINDING(vmaFreeMemory)
      void
      clear() VULKAN_HPP_NOEXCEPT {
        if (**this) {
          getAllocator().freeMemory(*this);
          release();
        }
      }

      VMA_HPP_BINDING(vmaGetAllocationInfo)
      VULKAN_HPP_NODISCARD AllocationInfo
      getInfo() const VULKAN_HPP_NOEXCEPT {
        return getAllocator().getAllocationInfo(*this);
      }

      VMA_HPP_BINDING(vmaGetAllocationInfo2)
      VULKAN_HPP_NODISCARD AllocationInfo2
      getInfo2() const VULKAN_HPP_NOEXCEPT {
        return getAllocator().getAllocationInfo2(*this);
      }

      VMA_HPP_BINDING(vmaSetAllocationUserData)
      void
      setUserData(void* userData) const VULKAN_HPP_NOEXCEPT {
        getAllocator().setAllocationUserData(*this, userData);
      }

      VMA_HPP_BINDING(vmaSetAllocationName)
      void
      setName(const char* name) const VULKAN_HPP_NOEXCEPT {
        getAllocator().setAllocationName(*this, name);
      }

      VMA_HPP_BINDING(vmaGetAllocationMemoryProperties)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryPropertyFlags
      getMemoryProperties() const VULKAN_HPP_NOEXCEPT {
        return getAllocator().getAllocationMemoryProperties(*this);
      }

#if VMA_EXTERNAL_MEMORY_WIN32
      VMA_HPP_BINDING(vmaGetMemoryWin32Handle)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<HANDLE>::type
      getWin32Handle(HANDLE hTargetProcess) const {
        return getAllocator().getMemoryWin32Handle(*this, hTargetProcess);
      }
#endif

      VMA_HPP_BINDING(vmaMapMemory)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<void*>::type
      map() const {
        return getAllocator().mapMemory(*this);
      }

      VMA_HPP_BINDING(vmaUnmapMemory)
      void
      unmap() const VULKAN_HPP_NOEXCEPT {
        getAllocator().unmapMemory(*this);
      }

      VMA_HPP_BINDING(vmaFlushAllocation)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      flush(VULKAN_HPP_NAMESPACE::DeviceSize offset, VULKAN_HPP_NAMESPACE::DeviceSize size) const {
        return getAllocator().flushAllocation(*this, offset, size);
      }

      VMA_HPP_BINDING(vmaInvalidateAllocation)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      invalidate(VULKAN_HPP_NAMESPACE::DeviceSize offset, VULKAN_HPP_NAMESPACE::DeviceSize size) const {
        return getAllocator().invalidateAllocation(*this, offset, size);
      }

      VMA_HPP_BINDING(vmaCopyMemoryToAllocation)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      copyFromMemory(const void* srcHostPointer,
                     VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                     VULKAN_HPP_NAMESPACE::DeviceSize size) const {
        return getAllocator().copyMemoryToAllocation(srcHostPointer, *this, dstAllocationLocalOffset, size);
      }

      VMA_HPP_BINDING(vmaCopyAllocationToMemory)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      copyToMemory(VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                   void* dstHostPointer,
                   VULKAN_HPP_NAMESPACE::DeviceSize size) const {
        return getAllocator().copyAllocationToMemory(*this, srcAllocationLocalOffset, dstHostPointer, size);
      }

      VMA_HPP_BINDING(vmaBindBufferMemory)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      bindBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer) const {
        return getAllocator().bindBufferMemory(*this, buffer);
      }

      VMA_HPP_BINDING(vmaBindBufferMemory2)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      bindBuffer2(VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                  VULKAN_HPP_NAMESPACE::Buffer buffer,
                  const void* pNext) const {
        return getAllocator().bindBufferMemory2(*this, allocationLocalOffset, buffer, pNext);
      }

      VMA_HPP_BINDING(vmaBindImageMemory)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      bindImage(VULKAN_HPP_NAMESPACE::Image image) const {
        return getAllocator().bindImageMemory(*this, image);
      }

      VMA_HPP_BINDING(vmaBindImageMemory2)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      bindImage2(VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                 VULKAN_HPP_NAMESPACE::Image image,
                 const void* pNext) const {
        return getAllocator().bindImageMemory2(*this, allocationLocalOffset, image, pNext);
      }
    };

    class DefragmentationContext : public detail::RAII<DefragmentationContext, 1,
        VMA_HPP_NAMESPACE::Allocator,
        VMA_HPP_NAMESPACE::DefragmentationContext> {
    public:
      using RAII::RAII;
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaBeginDefragmentation)
      DefragmentationContext(const VMA_HPP_NAMESPACE::Allocator& allocator, const DefragmentationInfo& info) :
        DefragmentationContext(allocator, allocator.beginDefragmentation(info)) {}
#endif

      VMA_HPP_NAMESPACE::Allocator getAllocator() const VULKAN_HPP_NOEXCEPT { return std::get<0>(*this); }

      VMA_HPP_BINDING(vmaEndDefragmentation)
      void
      clear() VULKAN_HPP_NOEXCEPT { end(); }

      VMA_HPP_BINDING(vmaEndDefragmentation)
      void
      end(VULKAN_HPP_NAMESPACE::Optional<DefragmentationStats> stats = nullptr) VULKAN_HPP_NOEXCEPT {
        if (**this) {
          getAllocator().endDefragmentation(*this, stats);
          release();
        }
      }

      VMA_HPP_BINDING(vmaBeginDefragmentationPass)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo>
      beginPass() const {
        DefragmentationPassMoveInfo moveInfo;
        VULKAN_HPP_NAMESPACE::Result result = getAllocator().beginDefragmentationPass(*this, &moveInfo);
        VMA_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_RAII_NAMESPACE_STRING "::DefragmentationContext::beginPass", {
          VULKAN_HPP_NAMESPACE::Result::eSuccess, VULKAN_HPP_NAMESPACE::Result::eIncomplete });
        return VULKAN_HPP_NAMESPACE::ResultValue(result, std::move(moveInfo));
      }

      VMA_HPP_BINDING(vmaEndDefragmentationPass)
      VULKAN_HPP_NAMESPACE::Result
      endPass(DefragmentationPassMoveInfo& passInfo) const VULKAN_HPP_NOEXCEPT {
        VULKAN_HPP_NAMESPACE::Result result = getAllocator().endDefragmentationPass(*this, &passInfo);
        VMA_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_RAII_NAMESPACE_STRING "::DefragmentationContext::endPass", {
          VULKAN_HPP_NAMESPACE::Result::eSuccess, VULKAN_HPP_NAMESPACE::Result::eIncomplete });
        return result;
      }
    };

    class Allocator : public detail::RAII<Allocator, 1,
        VULKAN_HPP_NAMESPACE::Device,
        VMA_HPP_NAMESPACE::Allocator,
        const VULKAN_HPP_NAMESPACE::AllocationCallbacks *,
        VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const *> {

      static VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type
      validationError(const char* message) {
#ifdef VULKAN_HPP_NO_EXCEPTIONS
        return VULKAN_HPP_NAMESPACE::ResultValue(VULKAN_HPP_NAMESPACE::Result::eErrorValidationFailed, Allocator(nullptr));
#else
        throw VULKAN_HPP_NAMESPACE::ValidationFailedError(message);
#endif
      }

      friend VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type
      createAllocator(VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Instance const & instance,
                      VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const & device,
                      AllocatorCreateInfo createInfo) {
        if (createInfo.instance) return validationError("createInfo.instance must be null");
        if (createInfo.device) return validationError("createInfo.device must be null");
        if (createInfo.pVulkanFunctions) return validationError("createInfo.pVulkanFunctions must be null");
        createInfo.instance = instance;
        createInfo.device = device;
        VulkanFunctions functions = functionsFromDispatcher(instance.getDispatcher(), device.getDispatcher());
        createInfo.pVulkanFunctions = &functions;
        return detail::util::wrap<Allocator>(device, createAllocator(createInfo), createInfo.pAllocationCallbacks);
      }

    public:
      using RAII::RAII;
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaCreateAllocator)
      Allocator(VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Instance const & instance,
                VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const & device,
                AllocatorCreateInfo const & createInfo) :
        Allocator(createAllocator(instance, device, createInfo)) {}
#endif

      explicit Allocator(VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Device const & device,
                         VMA_HPP_NAMESPACE::Allocator const & allocator,
                         const VULKAN_HPP_NAMESPACE::AllocationCallbacks * callbacks) VULKAN_HPP_NOEXCEPT :
        RAII(device, allocator, callbacks, device.getDispatcher()) {}

      VULKAN_HPP_NAMESPACE::Device getDevice() const VULKAN_HPP_NOEXCEPT { return std::get<0>(*this); }

      const VULKAN_HPP_NAMESPACE::AllocationCallbacks * getAllocationCallbacks() const VULKAN_HPP_NOEXCEPT { return std::get<2>(*this); }

      VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const * getDispatcher() const VULKAN_HPP_NOEXCEPT { return std::get<3>(*this); }

      VMA_HPP_BINDING(vmaDestroyAllocator)
      void
      clear() VULKAN_HPP_NOEXCEPT {
        if (**this) {
          (**this).destroy();
          release();
        }
      }

      VMA_HPP_BINDING(vmaGetAllocatorInfo)
      VULKAN_HPP_NODISCARD AllocatorInfo
      getAllocatorInfo() const VULKAN_HPP_NOEXCEPT {
        return (**this).getAllocatorInfo();
      }

      VMA_HPP_BINDING(vmaGetPhysicalDeviceProperties)
      VULKAN_HPP_NODISCARD const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties*
      getPhysicalDeviceProperties() const VULKAN_HPP_NOEXCEPT {
        return (**this).getPhysicalDeviceProperties();
      }

      VMA_HPP_BINDING(vmaGetMemoryProperties)
      VULKAN_HPP_NODISCARD const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties*
      getMemoryProperties() const VULKAN_HPP_NOEXCEPT {
        return (**this).getMemoryProperties();
      }

      VMA_HPP_BINDING(vmaGetMemoryTypeProperties)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryPropertyFlags
      getMemoryTypeProperties(uint32_t memoryTypeIndex) const VULKAN_HPP_NOEXCEPT {
        return (**this).getMemoryTypeProperties(memoryTypeIndex);
      }

      VMA_HPP_BINDING(vmaSetCurrentFrameIndex)
      void
      setCurrentFrameIndex(uint32_t frameIndex) const VULKAN_HPP_NOEXCEPT {
        (**this).setCurrentFrameIndex(frameIndex);
      }

      VMA_HPP_BINDING(vmaCalculateStatistics)
      VULKAN_HPP_NODISCARD TotalStatistics
      calculateStatistics() const VULKAN_HPP_NOEXCEPT {
        return (**this).calculateStatistics();
      }

      VMA_HPP_BINDING(vmaGetHeapBudgets)
      VULKAN_HPP_NODISCARD std::vector<Budget>
      getHeapBudgets() const VULKAN_HPP_NOEXCEPT {
        return (**this).getHeapBudgets();
      }

      VMA_HPP_BINDING(vmaFindMemoryTypeIndex)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type
      findMemoryTypeIndex(uint32_t memoryTypeBits, const AllocationCreateInfo& allocationCreateInfo) const {
        return (**this).findMemoryTypeIndex(memoryTypeBits, allocationCreateInfo);
      }

      VMA_HPP_BINDING(vmaFindMemoryTypeIndexForBufferInfo)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type
      findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                       const AllocationCreateInfo& allocationCreateInfo) const {
        return (**this).findMemoryTypeIndexForBufferInfo(bufferCreateInfo, allocationCreateInfo);
      }

      VMA_HPP_BINDING(vmaFindMemoryTypeIndexForImageInfo)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type
      findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                      const AllocationCreateInfo& allocationCreateInfo) const {
        return (**this).findMemoryTypeIndexForImageInfo(imageCreateInfo, allocationCreateInfo);
      }

      VMA_HPP_BINDING(vmaCreatePool)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type
      createPool(const PoolCreateInfo& createInfo) const {
        return detail::util::wrap<Pool>(*this, (**this).createPool(createInfo));
      }

      VMA_HPP_BINDING(vmaAllocateMemory)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type
      allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements& memoryRequirements,
                     const AllocationCreateInfo& createInfo,
                     VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const {
        return detail::util::wrap<Allocation>(*this, (**this).allocateMemory(memoryRequirements, createInfo, allocationInfo));
      }

      VMA_HPP_BINDING(vmaAllocateMemoryPages)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation>>::type
      allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> memoryRequirements,
                          VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                          VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo = nullptr) const {
        VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<VMA_HPP_NAMESPACE::Allocation>>::type allocations =
          (**this).allocateMemoryPages(memoryRequirements, createInfo, allocationInfo);
#ifdef VULKAN_HPP_NO_EXCEPTIONS
#define allocations allocations.value
#endif
        std::vector<Allocation> allocationsRAII;
        allocationsRAII.reserve(allocations.size());
        for (const VMA_HPP_NAMESPACE::Allocation& allocation : allocations) {
          if (!allocation) allocationsRAII.emplace_back(nullptr);
          else allocationsRAII.emplace_back(*this, allocation);
        }
#ifdef VULKAN_HPP_NO_EXCEPTIONS
#undef allocations
        return VULKAN_HPP_NAMESPACE::ResultValue<std::vector<Allocation>>(allocations.result, std::move(allocationsRAII));
#else
        return allocationsRAII;
#endif
      }

      VMA_HPP_BINDING(vmaAllocateMemoryForBuffer)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type
      allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                              const AllocationCreateInfo& createInfo,
                              VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const {
        return detail::util::wrap<Allocation>(*this, (**this).allocateMemoryForBuffer(buffer, createInfo, allocationInfo));
      }

      VMA_HPP_BINDING(vmaAllocateMemoryForImage)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type
      allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                             const AllocationCreateInfo& createInfo,
                             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const {
        return detail::util::wrap<Allocation>(*this, (**this).allocateMemoryForImage(image, createInfo, allocationInfo));
      }

      VMA_HPP_BINDING(vmaFreeMemoryPages)
      void
      freeMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VMA_HPP_NAMESPACE::Allocation> allocations) const VULKAN_HPP_NOEXCEPT {
        (**this).freeMemoryPages(allocations);
      }

      VMA_HPP_BINDING(vmaFlushAllocations)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      flushAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const VMA_HPP_NAMESPACE::Allocation> allocations,
                       VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                       VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes) const {
        return (**this).flushAllocations(allocations, offsets, sizes);
      }

      VMA_HPP_BINDING(vmaInvalidateAllocations)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      invalidateAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const VMA_HPP_NAMESPACE::Allocation> allocations,
                            VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                            VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes) const {
        return (**this).invalidateAllocations(allocations, offsets, sizes);
      }

      VMA_HPP_BINDING(vmaCheckCorruption)
      VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
      checkCorruption(uint32_t memoryTypeBits) const {
        return (**this).checkCorruption(memoryTypeBits);
      }

      VMA_HPP_BINDING(vmaBeginDefragmentation)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationContext>::type
      beginDefragmentation(const DefragmentationInfo& info) const {
        return detail::util::wrap<DefragmentationContext>(*this, (**this).beginDefragmentation(info));
      }

      VMA_HPP_BINDING(vmaCreateBuffer)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
      createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                   const AllocationCreateInfo& allocationCreateInfo,
                   VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;

      VMA_HPP_BINDING(vmaCreateBufferWithAlignment)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
      createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                const AllocationCreateInfo& allocationCreateInfo,
                                VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;

      VMA_HPP_BINDING(vmaCreateAliasingBuffer)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
      createAliasingBuffer(Allocation&& allocation,
                           const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateAliasingBuffer2)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
      createAliasingBuffer2(Allocation&& allocation,
                            VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                            const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateAliasingBuffer)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type
      createAliasingBuffer(VMA_HPP_NAMESPACE::Allocation allocation,
                           const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateAliasingBuffer2)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type
      createAliasingBuffer2(VMA_HPP_NAMESPACE::Allocation allocation,
                            VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                            const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateImage)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type
      createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                  const AllocationCreateInfo& allocationCreateInfo,
                  VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;

      VMA_HPP_BINDING(vmaCreateAliasingImage)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type
      createAliasingImage(Allocation&& allocation,
                          const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateAliasingImage2)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type
      createAliasingImage2(Allocation&& allocation,
                           VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                           const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateAliasingImage)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type
      createAliasingImage(VMA_HPP_NAMESPACE::Allocation allocation,
                          const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const;

      VMA_HPP_BINDING(vmaCreateAliasingImage2)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type
      createAliasingImage2(VMA_HPP_NAMESPACE::Allocation allocation,
                           VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                           const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const;

#if VMA_STATS_STRING_ENABLED
      VMA_HPP_BINDING(vmaBuildStatsString)
      VMA_HPP_BINDING(vmaFreeStatsString)
      VULKAN_HPP_NODISCARD StatsString
      buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT {
        return StatsString::wrap<VMA_HPP_NAMESPACE::Allocator, &vmaFreeStatsString>(*this, (**this).buildStatsString(detailedMap));
      }
#endif
    };

    class VirtualAllocation : public detail::RAII<VirtualAllocation, 1,
        VMA_HPP_NAMESPACE::VirtualBlock,
        VMA_HPP_NAMESPACE::VirtualAllocation> {
    public:
      using RAII::RAII;
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaVirtualAllocate)
      VirtualAllocation(const VMA_HPP_NAMESPACE::VirtualBlock& virtualBlock,
                        const VirtualAllocationCreateInfo& createInfo,
                        VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset = nullptr) :
        RAII(virtualBlock, virtualBlock.virtualAllocate(createInfo, offset)) {}
#endif

      VMA_HPP_NAMESPACE::VirtualBlock getVirtualBlock() const VULKAN_HPP_NOEXCEPT { return std::get<0>(*this); }

      VMA_HPP_BINDING(vmaVirtualFree)
      void
      clear() VULKAN_HPP_NOEXCEPT {
        if (**this) {
          getVirtualBlock().virtualFree(*this);
          release();
        }
      }

      VMA_HPP_BINDING(vmaGetVirtualAllocationInfo)
      VULKAN_HPP_NODISCARD VirtualAllocationInfo
      getInfo() const {
        return getVirtualBlock().getVirtualAllocationInfo(*this);
      }

      VMA_HPP_BINDING(vmaSetVirtualAllocationUserData)
      void
      setUserData(void* userData) const {
        getVirtualBlock().setVirtualAllocationUserData(*this, userData);
      }
    };

    class VirtualBlock : public detail::RAII<VirtualBlock, 0,
        VMA_HPP_NAMESPACE::VirtualBlock> {

      friend class detail::util;
      // Private helper constructor for detail::util::wrap
      VirtualBlock(std::nullptr_t, VMA_HPP_NAMESPACE::VirtualBlock virtualBlock) VULKAN_HPP_NOEXCEPT : RAII(virtualBlock) {}

      friend VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type
      createVirtualBlock(const VirtualBlockCreateInfo& createInfo) {
        return detail::util::wrap<VirtualBlock>(nullptr, VMA_HPP_NAMESPACE::createVirtualBlock(createInfo));
      }

    public:
      using RAII::RAII;
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaCreateVirtualBlock)
      explicit VirtualBlock(const VirtualBlockCreateInfo& createInfo) :
        RAII(VMA_HPP_NAMESPACE::createVirtualBlock(createInfo)) {}
#endif

      VMA_HPP_BINDING(vmaDestroyVirtualBlock)
      void
      clear() VULKAN_HPP_NOEXCEPT {
        if (**this) {
          (**this).destroy();
          release();
        }
      }

      VMA_HPP_BINDING(vmaIsVirtualBlockEmpty)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Bool32
      isEmpty() const {
        return (**this).isVirtualBlockEmpty();
      }

      VMA_HPP_BINDING(vmaVirtualAllocate)
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValueType<VirtualAllocation>::type
      virtualAllocate(const VirtualAllocationCreateInfo& createInfo,
                      VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset = nullptr) const {
        return detail::util::wrap<VirtualAllocation>(*this, (**this).virtualAllocate(createInfo, offset));
      }

      VMA_HPP_BINDING(vmaClearVirtualBlock)
      void
      clearBlock() const {
        (**this).clearVirtualBlock();
      }

      VMA_HPP_BINDING(vmaGetVirtualBlockStatistics)
      VULKAN_HPP_NODISCARD Statistics
      getStatistics() const {
        return (**this).getVirtualBlockStatistics();
      }

      VMA_HPP_BINDING(vmaCalculateVirtualBlockStatistics)
      VULKAN_HPP_NODISCARD DetailedStatistics
      calculateStatistics() const {
        return (**this).calculateVirtualBlockStatistics();
      }

#if VMA_STATS_STRING_ENABLED
      VMA_HPP_BINDING(vmaBuildVirtualBlockStatsString)
      VMA_HPP_BINDING(vmaFreeVirtualBlockStatsString)
      VULKAN_HPP_NODISCARD StatsString
      buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const {
        return StatsString::wrap<VMA_HPP_NAMESPACE::VirtualBlock, &vmaFreeVirtualBlockStatsString>(*this, (**this).buildVirtualBlockStatsString(detailedMap));
      }
#endif
    };

    namespace detail {
      template<class T> class RAIIResource : public T {
        friend class util;
      public:
        using CppType = T::CppType;
        using CType   = CppType::CType;

        RAIIResource(Allocation&& allocation, T&& resource) VULKAN_HPP_NOEXCEPT :
          T(std::move(resource)), m_allocation(std::move(allocation)) {}

        RAIIResource(std::nullptr_t) VULKAN_HPP_NOEXCEPT : T(nullptr), m_allocation(nullptr) {}

        ~RAIIResource() { this->clear(); }

        RAIIResource() VULKAN_HPP_NOEXCEPT = delete;
        RAIIResource(RAIIResource const&) VULKAN_HPP_NOEXCEPT = delete;
        RAIIResource& operator=(RAIIResource const &) VULKAN_HPP_NOEXCEPT = delete;

        RAIIResource(RAIIResource && rhs) VULKAN_HPP_NOEXCEPT :
          T(std::exchange((T&) rhs, nullptr)), m_allocation(std::exchange(rhs.m_allocation, nullptr)) {}
        RAIIResource& operator=(RAIIResource && rhs) VULKAN_HPP_NOEXCEPT {
          if (this != &rhs) {
            std::swap((T&) *this, (T&) rhs);
            std::swap(m_allocation, rhs.m_allocation);
          }
          return *this;
        }

        CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return T::operator*(); }
        CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return T::operator*(); }
        operator CppType() const VULKAN_HPP_NOEXCEPT { return T::operator CppType(); }

        void clear() VULKAN_HPP_NOEXCEPT {
          T::clear();
          m_allocation.clear();
        }

        std::pair<Allocation, T> split() VULKAN_HPP_NOEXCEPT {
          return { std::move(m_allocation), std::move(*this) };
        }

        std::pair<VMA_HPP_NAMESPACE::Allocation, CppType> release() VULKAN_HPP_NOEXCEPT {
          return { m_allocation.release(), T::release() };
        }

        const Allocation& getAllocation() const VULKAN_HPP_NOEXCEPT { return m_allocation; }

        void swap(RAIIResource & rhs) VULKAN_HPP_NOEXCEPT {
          std::swap((T&) *this, (T&) rhs);
          std::swap(m_allocation, rhs.m_allocation);
        }

      protected:
        RAIIResource(const Allocator& allocator, std::pair<VMA_HPP_NAMESPACE::Allocation, CppType> pair) VULKAN_HPP_NOEXCEPT :
          RAIIResource(allocator, pair.second, Allocation(allocator, pair.first)) {}

      private:
        RAIIResource(const Allocator& allocator, CppType handle, Allocation&& allocation = nullptr) VULKAN_HPP_NOEXCEPT = delete;

        Allocation m_allocation;
      };

      template<> VULKAN_HPP_INLINE RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::
      RAIIResource(const Allocator& allocator, VULKAN_HPP_NAMESPACE::Buffer handle, Allocation&& allocation) VULKAN_HPP_NOEXCEPT :
        Buffer(nullptr), m_allocation(std::move(allocation)) {
        m_device = allocator.getDevice();
        m_buffer = handle;
        m_allocator = allocator.getAllocationCallbacks();
        m_dispatcher = allocator.getDispatcher();
      }

      template<> VULKAN_HPP_INLINE RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::
      RAIIResource(const Allocator& allocator, VULKAN_HPP_NAMESPACE::Image handle, Allocation&& allocation) VULKAN_HPP_NOEXCEPT :
        Image(nullptr), m_allocation(std::move(allocation)) {
        m_device = allocator.getDevice();
        m_image = handle;
        m_allocator = allocator.getAllocationCallbacks();
        m_dispatcher = allocator.getDispatcher();
      }
    }
    
    class Buffer : public detail::RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer> {
    public:
      using RAIIResource::RAIIResource;
      
      VMA_HPP_BINDING(vmaBindBufferMemory)
      static Buffer bind(Allocation&& allocation, Buffer&& buffer) {
        allocation.bindBuffer(buffer);
        return Buffer(std::move(allocation), std::move(buffer));
      }
      
      VMA_HPP_BINDING(vmaBindBufferMemory2)
      static Buffer bind(Allocation&& allocation,
                         VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                         Buffer&& buffer,
                         const void* pNext) {
        allocation.bindBuffer2(allocationLocalOffset, buffer, pNext);
        return Buffer(std::move(allocation), std::move(buffer));
      }

#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      
      VMA_HPP_BINDING(vmaCreateBuffer)
      Buffer(const Allocator& allocator,
             const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
             const AllocationCreateInfo& allocationCreateInfo,
             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) :
        Buffer(allocator, (*allocator).createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo)) {}
      
      VMA_HPP_BINDING(vmaCreateBufferWithAlignment)
      Buffer(const Allocator& allocator,
             const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
             const AllocationCreateInfo& allocationCreateInfo,
             VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) :
        Buffer(allocator, (*allocator).createBufferWithAlignment(bufferCreateInfo, allocationCreateInfo, minAlignment, allocationInfo)) {}
      
      VMA_HPP_BINDING(vmaCreateAliasingBuffer)
      Buffer(const Allocator& allocator,
             Allocation&& allocation,
             const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) :
        Buffer(std::move(allocation), allocator.createAliasingBuffer(allocation, bufferCreateInfo)) {}
      
      VMA_HPP_BINDING(vmaCreateAliasingBuffer2)
      Buffer(const Allocator& allocator,
             Allocation&& allocation,
             VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
             const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) :
        Buffer(std::move(allocation), allocator.createAliasingBuffer2(allocation, allocationLocalOffset, bufferCreateInfo)) {}
#endif
      
      VMA_HPP_BINDING(vmaDestroyBuffer) using RAIIResource::clear;
    };

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
    Allocator::createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                            const AllocationCreateInfo& allocationCreateInfo,
                            VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
      return detail::util::wrap<Buffer>(*this,
        (**this).createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
    Allocator::createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                         const AllocationCreateInfo& allocationCreateInfo,
                                         VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                         VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
      return detail::util::wrap<Buffer>(*this,
        (**this).createBufferWithAlignment(bufferCreateInfo, allocationCreateInfo, minAlignment, allocationInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
    Allocator::createAliasingBuffer(Allocation&& allocation,
                                    const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const {
      return detail::util::wrap<Buffer>(std::move(allocation), this->createAliasingBuffer(allocation, bufferCreateInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type
    Allocator::createAliasingBuffer2(Allocation&& allocation,
                                     VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                     const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const {
      return detail::util::wrap<Buffer>(std::move(allocation), this->createAliasingBuffer2(allocation, allocationLocalOffset, bufferCreateInfo));
    }

      VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type
      Allocator::createAliasingBuffer(VMA_HPP_NAMESPACE::Allocation allocation,
                                      const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const {
      return detail::util::wrap<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer,
                                detail::RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>>(
        *this, (**this).createAliasingBuffer(allocation, bufferCreateInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type
    Allocator::createAliasingBuffer2(VMA_HPP_NAMESPACE::Allocation allocation,
                                     VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                     const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const {
      return detail::util::wrap<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer,
                                detail::RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>>(
        *this, (**this).createAliasingBuffer2(allocation, allocationLocalOffset, bufferCreateInfo));
    }

    class Image : public detail::RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image> {
    public:
      using RAIIResource::RAIIResource;

      VMA_HPP_BINDING(vmaBindImageMemory)
      static Image bind(Allocation&& allocation, Image&& image) {
        allocation.bindImage(image);
        return Image(std::move(allocation), std::move(image));
      }

      VMA_HPP_BINDING(vmaBindImageMemory2)
      static Image bind(Allocation&& allocation,
                        VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                        Image&& image,
                        const void* pNext) {
        allocation.bindImage2(allocationLocalOffset, image, pNext);
        return Image(std::move(allocation), std::move(image));
      }

#if !defined( VULKAN_HPP_NO_EXCEPTIONS )

      VMA_HPP_BINDING(vmaCreateImage)
      Image(const Allocator& allocator,
            const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
            const AllocationCreateInfo& allocationCreateInfo,
            VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) :
        Image(allocator, (*allocator).createImage(imageCreateInfo, allocationCreateInfo, allocationInfo)) {}

      VMA_HPP_BINDING(vmaCreateAliasingImage)
      Image(const Allocator& allocator,
            Allocation&& allocation,
            const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) :
        Image(std::move(allocation), allocator.createAliasingImage(allocation, imageCreateInfo)) {}

      VMA_HPP_BINDING(vmaCreateAliasingImage2)
      Image(const Allocator& allocator,
            Allocation&& allocation,
            VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
            const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) :
        Image(std::move(allocation), allocator.createAliasingImage2(allocation, allocationLocalOffset, imageCreateInfo)) {}
#endif

      VMA_HPP_BINDING(vmaDestroyImage) using RAIIResource::clear;
    };

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type
    Allocator::createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                               const AllocationCreateInfo& allocationCreateInfo,
                               VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
      return detail::util::wrap<Image>(*this,
        (**this).createImage(imageCreateInfo, allocationCreateInfo, allocationInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type
    Allocator::createAliasingImage(Allocation&& allocation,
                                       const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const {
      return detail::util::wrap<Image>(std::move(allocation), this->createAliasingImage(allocation, imageCreateInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type
    Allocator::createAliasingImage2(Allocation&& allocation,
                                        VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                        const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const {
      return detail::util::wrap<Image>(std::move(allocation), this->createAliasingImage2(allocation, allocationLocalOffset, imageCreateInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type
    Allocator::createAliasingImage(VMA_HPP_NAMESPACE::Allocation allocation,
                                   const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const {
      return detail::util::wrap<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image,
                                detail::RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>>(
        *this, (**this).createAliasingImage(allocation, imageCreateInfo));
    }

    VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type
    Allocator::createAliasingImage2(VMA_HPP_NAMESPACE::Allocation allocation,
                                    VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                    const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const {
      return detail::util::wrap<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image,
                                detail::RAIIResource<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>>(
        *this, (**this).createAliasingImage2(allocation, allocationLocalOffset, imageCreateInfo));
    }

    VMA_HPP_VALIDATE(
      Pool,
      Allocation,
      DefragmentationContext,
      Allocator,
      VirtualAllocation,
      VirtualBlock,
      Buffer,
      Image
    );
  }
}
#endif
#endif
