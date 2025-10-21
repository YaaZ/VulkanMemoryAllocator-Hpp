// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
#ifndef VULKAN_MEMORY_ALLOCATOR_RAII_HPP
#define VULKAN_MEMORY_ALLOCATOR_RAII_HPP

namespace VMA_HPP_NAMESPACE {
  namespace VMA_HPP_RAII_NAMESPACE {
    class Allocator;
    class Pool;
    class Allocation;
    class DefragmentationContext;
    class VirtualAllocation;
    class VirtualBlock;
    class Buffer;
    class Image;
#if VMA_STATS_STRING_ENABLED
    class StatsString;
#endif 
    using VULKAN_HPP_NAMESPACE::exchange;

    // wrapper class for handle VmaAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocator.html
    class Allocator {
    public:
      using CType   = VmaAllocator;
      using CppType = VMA_HPP_NAMESPACE::Allocator;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      Allocator(std::nullptr_t) {}
      ~Allocator() { clear(); }

      Allocator() = delete;
      Allocator(Allocator const &) = delete;

      Allocator(Allocator && rhs) VULKAN_HPP_NOEXCEPT
        : m_device(exchange(rhs.m_device, {}))
        , m_allocator(exchange(rhs.m_allocator, {}))
        , m_allocationCallbacks(exchange(rhs.m_allocationCallbacks, {}))
        , m_dispatcher(exchange(rhs.m_dispatcher, {})) {}

      Allocator& operator=(Allocator const &) = delete;
      Allocator& operator=(Allocator && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_device, rhs.m_device);
          std::swap(m_allocator, rhs.m_allocator);
          std::swap(m_allocationCallbacks, rhs.m_allocationCallbacks);
          std::swap(m_dispatcher, rhs.m_dispatcher);
        }
        return *this;
      }

      CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_allocator; }
      CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_allocator); }
      operator CppType() const VULKAN_HPP_NOEXCEPT { return m_allocator; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_allocator) m_allocator.destroy();
        m_device = nullptr;
        m_allocator = nullptr;
        m_allocationCallbacks = nullptr;
        m_dispatcher = nullptr;
      }

      CppType release() {
        m_device = nullptr;
        m_allocationCallbacks = nullptr;
        m_dispatcher = nullptr;
        return exchange(m_allocator, nullptr);
      }

      VULKAN_HPP_NAMESPACE::Device getDevice() const { return m_device; }

      const VULKAN_HPP_NAMESPACE::AllocationCallbacks * getAllocationCallbacks() const { return m_allocationCallbacks; }

      VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const * getDispatcher() const { return m_dispatcher; }

      void swap(Allocator & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_device, rhs.m_device);
        std::swap(m_allocator, rhs.m_allocator);
        std::swap(m_allocationCallbacks, rhs.m_allocationCallbacks);
        std::swap(m_dispatcher, rhs.m_dispatcher);
      }

      // wrapper function for command vmaGetAllocatorInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD AllocatorInfo getInfo() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetPhysicalDeviceProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* getPhysicalDeviceProperties() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* getMemoryProperties() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetMemoryTypeProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getMemoryTypeProperties(uint32_t memoryTypeIndex) const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaSetCurrentFrameIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void setCurrentFrameIndex(uint32_t frameIndex) const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaCalculateStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD TotalStatistics calculateStatistics() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD std::vector<Budget> getHeapBudgets() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaFindMemoryTypeIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                                              const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaFindMemoryTypeIndexForBufferInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                           const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaFindMemoryTypeIndexForImageInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                          const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type createPool(const PoolCreateInfo& createInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                           const AllocationCreateInfo& createInfo,
                                                                                                           VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation>>::type allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                             VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                             VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                    const AllocationCreateInfo& createInfo,
                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                   const AllocationCreateInfo& createInfo,
                                                                                                                   VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void freeMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations) const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaFlushAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flushAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations,
                                                                                                                          VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & offsets,
                                                                                                                          VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & sizes) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaInvalidateAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidateAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations,
                                                                                                                               VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & offsets,
                                                                                                                               VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & sizes) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCheckCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkCorruption(uint32_t memoryTypeBits) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaBeginDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationContext>::type beginDefragmentation(const DefragmentationInfo& info) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                     const AllocationCreateInfo& allocationCreateInfo,
                                                                                                     VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Buffer>::type createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                  const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                  VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                  VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Image>::type createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                   const AllocationCreateInfo& allocationCreateInfo,
                                                                                                   VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

#if VMA_STATS_STRING_ENABLED
      // wrapper function for command vmaBuildStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD StatsString buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT;
#endif 

    private:
      VULKAN_HPP_NAMESPACE::Device m_device = {};
      VMA_HPP_NAMESPACE::Allocator m_allocator = {};
      const VULKAN_HPP_NAMESPACE::AllocationCallbacks * m_allocationCallbacks = {};
      VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const * m_dispatcher = {};
    };

    // wrapper class for handle VmaPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_pool.html
    class Pool {
    public:
      using CType   = VmaPool;
      using CppType = VMA_HPP_NAMESPACE::Pool;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      Pool(std::nullptr_t) {}
      ~Pool() { clear(); }

      Pool() = delete;
      Pool(Pool const &) = delete;

      Pool(Pool && rhs) VULKAN_HPP_NOEXCEPT
        : m_allocator(exchange(rhs.m_allocator, {}))
        , m_pool(exchange(rhs.m_pool, {})) {}

      Pool& operator=(Pool const &) = delete;
      Pool& operator=(Pool && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_allocator, rhs.m_allocator);
          std::swap(m_pool, rhs.m_pool);
        }
        return *this;
      }

      CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_pool; }
      CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_pool); }
      operator CppType() const VULKAN_HPP_NOEXCEPT { return m_pool; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_pool) m_allocator.destroyPool(m_pool);
        m_allocator = nullptr;
        m_pool = nullptr;
      }

      CppType release() {
        m_allocator = nullptr;
        return exchange(m_pool, nullptr);
      }

      VMA_HPP_NAMESPACE::Allocator getAllocator() const { return m_allocator; }

      void swap(Pool & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_allocator, rhs.m_allocator);
        std::swap(m_pool, rhs.m_pool);
      }

      // wrapper function for command vmaGetPoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD Statistics getStatistics() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaCalculatePoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD DetailedStatistics calculateStatistics() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaCheckPoolCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkCorruption() const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaGetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD const char* getName() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaSetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void setName(const char* name) const VULKAN_HPP_NOEXCEPT;

    private:
      VMA_HPP_NAMESPACE::Allocator m_allocator = {};
      VMA_HPP_NAMESPACE::Pool m_pool = {};
    };

    // wrapper class for handle VmaAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation.html
    class Allocation {
    public:
      using CType   = VmaAllocation;
      using CppType = VMA_HPP_NAMESPACE::Allocation;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      Allocation(std::nullptr_t) {}
      ~Allocation() { clear(); }

      Allocation() = delete;
      Allocation(Allocation const &) = delete;

      Allocation(Allocation && rhs) VULKAN_HPP_NOEXCEPT
        : m_allocator(exchange(rhs.m_allocator, {}))
        , m_allocation(exchange(rhs.m_allocation, {})) {}

      Allocation& operator=(Allocation const &) = delete;
      Allocation& operator=(Allocation && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_allocator, rhs.m_allocator);
          std::swap(m_allocation, rhs.m_allocation);
        }
        return *this;
      }

      CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_allocation; }
      CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_allocation); }
      operator CppType() const VULKAN_HPP_NOEXCEPT { return m_allocation; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_allocation) m_allocator.freeMemory(m_allocation);
        m_allocator = nullptr;
        m_allocation = nullptr;
      }

      CppType release() {
        m_allocator = nullptr;
        return exchange(m_allocation, nullptr);
      }

      VMA_HPP_NAMESPACE::Allocator getAllocator() const { return m_allocator; }

      void swap(Allocation & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_allocator, rhs.m_allocator);
        std::swap(m_allocation, rhs.m_allocation);
      }

      // wrapper function for command vmaGetAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD AllocationInfo getInfo() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetAllocationInfo2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD AllocationInfo2 getInfo2() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaSetAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void setUserData(void* userData) const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaSetAllocationName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void setName(const char* name) const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetAllocationMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getMemoryProperties() const VULKAN_HPP_NOEXCEPT;

#if VMA_EXTERNAL_MEMORY_WIN32
      // wrapper function for command vmaGetMemoryWin32Handle, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<HANDLE>::type getWin32Handle(HANDLE hTargetProcess) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

#endif 
      // wrapper function for command vmaMapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<void*>::type map() const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaUnmapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void unmap() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaFlushAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flush(VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                               VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaInvalidateAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidate(VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                                    VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCopyMemoryToAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type copyFromMemory(const void* srcHostPointer,
                                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCopyAllocationToMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type copyToMemory(VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                                                                                                                      void* dstHostPointer,
                                                                                                                      VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaBindBufferMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaBindBufferMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBuffer2(VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                     VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                     const void* next) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaBindImageMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImage(VULKAN_HPP_NAMESPACE::Image image) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaBindImageMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImage2(VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                    VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                    const void* next) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type createAliasingBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer>::type createAliasingBuffer2(VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                               const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type createAliasingImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image>::type createAliasingImage2(VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                             const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

    private:
      VMA_HPP_NAMESPACE::Allocator m_allocator = {};
      VMA_HPP_NAMESPACE::Allocation m_allocation = {};
    };

    // wrapper class for handle VmaDefragmentationContext, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_defragmentation_context.html
    class DefragmentationContext {
    public:
      using CType   = VmaDefragmentationContext;
      using CppType = VMA_HPP_NAMESPACE::DefragmentationContext;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      DefragmentationContext(std::nullptr_t) {}
      ~DefragmentationContext() { clear(); }

      DefragmentationContext() = delete;
      DefragmentationContext(DefragmentationContext const &) = delete;

      DefragmentationContext(DefragmentationContext && rhs) VULKAN_HPP_NOEXCEPT
        : m_allocator(exchange(rhs.m_allocator, {}))
        , m_defragmentationContext(exchange(rhs.m_defragmentationContext, {})) {}

      DefragmentationContext& operator=(DefragmentationContext const &) = delete;
      DefragmentationContext& operator=(DefragmentationContext && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_allocator, rhs.m_allocator);
          std::swap(m_defragmentationContext, rhs.m_defragmentationContext);
        }
        return *this;
      }

      CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_defragmentationContext; }
      CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_defragmentationContext); }
      operator CppType() const VULKAN_HPP_NOEXCEPT { return m_defragmentationContext; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_defragmentationContext) m_allocator.endDefragmentation(m_defragmentationContext);
        m_allocator = nullptr;
        m_defragmentationContext = nullptr;
      }

      CppType release() {
        m_allocator = nullptr;
        return exchange(m_defragmentationContext, nullptr);
      }

      VMA_HPP_NAMESPACE::Allocator getAllocator() const { return m_allocator; }

      void swap(DefragmentationContext & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_allocator, rhs.m_allocator);
        std::swap(m_defragmentationContext, rhs.m_defragmentationContext);
      }

      // wrapper function for command vmaEndDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void endDefragmentation(VULKAN_HPP_NAMESPACE::Optional<DefragmentationStats> stats = nullptr) const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaBeginDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo> beginDefragmentationPass() const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaEndDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo> endDefragmentationPass() const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

    private:
      VMA_HPP_NAMESPACE::Allocator m_allocator = {};
      VMA_HPP_NAMESPACE::DefragmentationContext m_defragmentationContext = {};
    };

    // wrapper class for handle VmaVirtualAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_allocation.html
    class VirtualAllocation {
    public:
      using CType   = VmaVirtualAllocation;
      using CppType = VMA_HPP_NAMESPACE::VirtualAllocation;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      VirtualAllocation(std::nullptr_t) {}
      ~VirtualAllocation() { clear(); }

      VirtualAllocation() = delete;
      VirtualAllocation(VirtualAllocation const &) = delete;

      VirtualAllocation(VirtualAllocation && rhs) VULKAN_HPP_NOEXCEPT
        : m_virtualBlock(exchange(rhs.m_virtualBlock, {}))
        , m_virtualAllocation(exchange(rhs.m_virtualAllocation, {})) {}

      VirtualAllocation& operator=(VirtualAllocation const &) = delete;
      VirtualAllocation& operator=(VirtualAllocation && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_virtualBlock, rhs.m_virtualBlock);
          std::swap(m_virtualAllocation, rhs.m_virtualAllocation);
        }
        return *this;
      }

      CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_virtualAllocation; }
      CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_virtualAllocation); }
      operator CppType() const VULKAN_HPP_NOEXCEPT { return m_virtualAllocation; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_virtualAllocation) m_virtualBlock.virtualFree(m_virtualAllocation);
        m_virtualBlock = nullptr;
        m_virtualAllocation = nullptr;
      }

      CppType release() {
        m_virtualBlock = nullptr;
        return exchange(m_virtualAllocation, nullptr);
      }

      VMA_HPP_NAMESPACE::VirtualBlock getVirtualBlock() const { return m_virtualBlock; }

      void swap(VirtualAllocation & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_virtualBlock, rhs.m_virtualBlock);
        std::swap(m_virtualAllocation, rhs.m_virtualAllocation);
      }

      // wrapper function for command vmaGetVirtualAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD VirtualAllocationInfo getInfo() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaSetVirtualAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void setUserData(void* userData) const VULKAN_HPP_NOEXCEPT;

    private:
      VMA_HPP_NAMESPACE::VirtualBlock m_virtualBlock = {};
      VMA_HPP_NAMESPACE::VirtualAllocation m_virtualAllocation = {};
    };

    // wrapper class for handle VmaVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_block.html
    class VirtualBlock {
    public:
      using CType   = VmaVirtualBlock;
      using CppType = VMA_HPP_NAMESPACE::VirtualBlock;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      VirtualBlock(std::nullptr_t) {}
      ~VirtualBlock() { clear(); }

      VirtualBlock() = delete;
      VirtualBlock(VirtualBlock const &) = delete;

      VirtualBlock(VirtualBlock && rhs) VULKAN_HPP_NOEXCEPT
        : m_virtualBlock(exchange(rhs.m_virtualBlock, {})) {}

      VirtualBlock& operator=(VirtualBlock const &) = delete;
      VirtualBlock& operator=(VirtualBlock && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_virtualBlock, rhs.m_virtualBlock);
        }
        return *this;
      }

      CppType const & operator*() const & VULKAN_HPP_NOEXCEPT { return m_virtualBlock; }
      CppType const && operator*() const && VULKAN_HPP_NOEXCEPT { return std::move(m_virtualBlock); }
      operator CppType() const VULKAN_HPP_NOEXCEPT { return m_virtualBlock; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_virtualBlock) m_virtualBlock.destroy();
        m_virtualBlock = nullptr;
      }

      CppType release() {
        return exchange(m_virtualBlock, nullptr);
      }

      void swap(VirtualBlock & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_virtualBlock, rhs.m_virtualBlock);
      }

      // wrapper function for command vmaIsVirtualBlockEmpty, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Bool32 isEmpty() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualAllocation>::type allocate(const VirtualAllocationCreateInfo& createInfo,
                                                                                                            VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

      // wrapper function for command vmaClearVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      void clearBlock() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaGetVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD Statistics getStatistics() const VULKAN_HPP_NOEXCEPT;

      // wrapper function for command vmaCalculateVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD DetailedStatistics calculateStatistics() const VULKAN_HPP_NOEXCEPT;

#if VMA_STATS_STRING_ENABLED
      // wrapper function for command vmaBuildVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
      VULKAN_HPP_NODISCARD StatsString buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT;
#endif 

    private:
      VMA_HPP_NAMESPACE::VirtualBlock m_virtualBlock = {};
    };

    // wrapper class for handle VkBuffer combined with VmaAllocation
    // see https://registry.khronos.org/vulkan/specs/latest/man/html/VkBuffer.html
    // see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation.html
    class Buffer : public VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer {
    public:
      using CType   = VkBuffer;
      using CppType = VULKAN_HPP_NAMESPACE::Buffer;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      Buffer(std::nullptr_t) : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer(nullptr) {}
      ~Buffer() { clear(); }

      Buffer() = delete;
      Buffer(Buffer const &) = delete;

      Buffer(Buffer && rhs) VULKAN_HPP_NOEXCEPT
        : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer(exchange(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer&>(rhs), nullptr))
        , m_allocation(exchange(rhs.m_allocation, nullptr)) {}

      Buffer& operator=(Buffer const &) = delete;
      Buffer& operator=(Buffer && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer&>(*this), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer&>(rhs));
          std::swap(m_allocation, rhs.m_allocation);
        }
        return *this;
      }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (*m_allocation) {
          const auto allocator = m_allocation.getAllocator();
          const auto pair = release();
          allocator.destroyBuffer(pair.second, pair.first);
        }
      }

      std::pair<Allocation, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer> split() VULKAN_HPP_NOEXCEPT {
        return { std::move(m_allocation), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer&&>(*this) };
      }

      std::pair<VMA_HPP_NAMESPACE::Allocation, VULKAN_HPP_NAMESPACE::Buffer> release() {
        return { m_allocation.release(), VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer::release() };
      }

      const Allocation& getAllocation() const { return m_allocation; }

      void swap(Buffer & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer&>(*this), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer&>(rhs));
        std::swap(m_allocation, rhs.m_allocation);
      }

    private:
      Allocation m_allocation = nullptr;
    };

    // wrapper class for handle VkImage combined with VmaAllocation
    // see https://registry.khronos.org/vulkan/specs/latest/man/html/VkImage.html
    // see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation.html
    class Image : public VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image {
    public:
      using CType   = VkImage;
      using CppType = VULKAN_HPP_NAMESPACE::Image;

    public:
#if !defined( VULKAN_HPP_NO_EXCEPTIONS )
      // TODO constructors
#endif

      // TODO raw constructor???

      Image(std::nullptr_t) : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image(nullptr) {}
      ~Image() { clear(); }

      Image() = delete;
      Image(Image const &) = delete;

      Image(Image && rhs) VULKAN_HPP_NOEXCEPT
        : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image(exchange(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image&>(rhs), nullptr))
        , m_allocation(exchange(rhs.m_allocation, nullptr)) {}

      Image& operator=(Image const &) = delete;
      Image& operator=(Image && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image&>(*this), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image&>(rhs));
          std::swap(m_allocation, rhs.m_allocation);
        }
        return *this;
      }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (*m_allocation) {
          const auto allocator = m_allocation.getAllocator();
          const auto pair = release();
          allocator.destroyImage(pair.second, pair.first);
        }
      }

      std::pair<Allocation, VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image> split() VULKAN_HPP_NOEXCEPT {
        return { std::move(m_allocation), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image&&>(*this) };
      }

      std::pair<VMA_HPP_NAMESPACE::Allocation, VULKAN_HPP_NAMESPACE::Image> release() {
        return { m_allocation.release(), VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image::release() };
      }

      const Allocation& getAllocation() const { return m_allocation; }

      void swap(Image & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image&>(*this), static_cast<VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image&>(rhs));
        std::swap(m_allocation, rhs.m_allocation);
      }

    private:
      Allocation m_allocation = nullptr;
    };

#if VMA_STATS_STRING_ENABLED
    // wrapper class for the stats string
    class StatsString {
    public:
      using CType   = char*;
      using CppType = char*;

    public:
      template<class Owner, void (*destructor)(Owner, char*)>
      static StatsString create(Owner owner, char* string) VULKAN_HPP_NOEXCEPT {
        StatsString result = nullptr;
        result.m_owner = static_cast<uint64_t>(owner);
        result.m_string = string;
        result.m_destructor = [](uint64_t owner, char* string) { destructor(static_cast<Owner>(owner), string); };
        return result;
      }

      StatsString(std::nullptr_t) {}
      ~StatsString() { clear(); }

      StatsString() = delete;
      StatsString(StatsString const &) = delete;

      StatsString(StatsString && rhs) VULKAN_HPP_NOEXCEPT
        : m_owner(exchange(rhs.m_owner, 0))
        , m_string(exchange(rhs.m_string, nullptr))
        , m_destructor(exchange(rhs.m_destructor, nullptr)) {}

      StatsString& operator=(StatsString const &) = delete;
      StatsString& operator=(StatsString && rhs) VULKAN_HPP_NOEXCEPT {
        if (this != &rhs) {
          std::swap(m_owner, rhs.m_owner);
          std::swap(m_string, rhs.m_string);
          std::swap(m_destructor, rhs.m_destructor);
        }
        return *this;
      }

      char* operator*() const VULKAN_HPP_NOEXCEPT { return m_string; }
      operator char*() const VULKAN_HPP_NOEXCEPT { return m_string; }

      void clear() VULKAN_HPP_NOEXCEPT {
        if (m_string) m_destructor(m_owner, m_string);
        m_owner = 0;
        m_string = nullptr;
        m_destructor = nullptr;
      }

      char* release() {
        m_owner = 0;
        m_destructor = nullptr;
        return exchange(m_string, nullptr);
      }

      void swap(StatsString & rhs) VULKAN_HPP_NOEXCEPT {
        std::swap(m_owner, rhs.m_owner);
        std::swap(m_string, rhs.m_string);
        std::swap(m_destructor, rhs.m_destructor);
      }

    private:
      uint64_t m_owner  = 0;
      char   * m_string = nullptr;
      void  (* m_destructor)(uint64_t, char*) = nullptr;
    };
#endif 

    // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type createAllocator(const AllocatorCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

    // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type createVirtualBlock(const VirtualBlockCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;

  }
}
#endif