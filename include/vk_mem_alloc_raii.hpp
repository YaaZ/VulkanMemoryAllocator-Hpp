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

    private:
      VULKAN_HPP_NAMESPACE::Device m_device;
      VMA_HPP_NAMESPACE::Allocator m_allocator;
      const VULKAN_HPP_NAMESPACE::AllocationCallbacks * m_allocationCallbacks;
      VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::detail::DeviceDispatcher const * m_dispatcher;
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

    private:
      VMA_HPP_NAMESPACE::Allocator m_allocator;
      VMA_HPP_NAMESPACE::Pool m_pool;
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

    private:
      VMA_HPP_NAMESPACE::Allocator m_allocator;
      VMA_HPP_NAMESPACE::Allocation m_allocation;
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

    private:
      VMA_HPP_NAMESPACE::Allocator m_allocator;
      VMA_HPP_NAMESPACE::DefragmentationContext m_defragmentationContext;
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

    private:
      VMA_HPP_NAMESPACE::VirtualBlock m_virtualBlock;
      VMA_HPP_NAMESPACE::VirtualAllocation m_virtualAllocation;
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

    private:
      VMA_HPP_NAMESPACE::VirtualBlock m_virtualBlock;
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

      Buffer(std::nullptr_t) : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Buffer(nullptr), m_allocation(nullptr) {}
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
      Allocation m_allocation;
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

      Image(std::nullptr_t) : VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::Image(nullptr), m_allocation(nullptr) {}
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
      Allocation m_allocation;
    };
  }
}
#endif