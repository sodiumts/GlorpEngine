#pragma once

#include "glorp_window.hpp"

// std lib headers
#include <vector>

namespace Glorp {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class GlorpDevice {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  GlorpDevice(GlorpWindow &window);
  ~GlorpDevice();

  // Not copyable or movable
  GlorpDevice(const GlorpDevice &) = delete;
  GlorpDevice &operator=(const GlorpDevice &) = delete;
  GlorpDevice(GlorpDevice &&) = delete;
  GlorpDevice &operator=(GlorpDevice &&) = delete;

  VkCommandPool getCommandPool() { return m_commandPool; }
  VkDevice device() { return m_device_; }
  VkSurfaceKHR surface() { return m_surface_; }
  VkQueue graphicsQueue() { return m_graphicsQueue_; }
  VkQueue presentQueue() { return m_presentQueue_; }
  VkInstance instance() { return m_instance; }
  VkPhysicalDevice physicalDevice() { return m_physicalDevice; }
  VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }


  SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_physicalDevice); }
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

  VkSampleCountFlagBits getSupportedSampleCount() { return m_msaaSamples; }

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);
  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  VkPhysicalDeviceProperties properties;

 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  VkSampleCountFlagBits getMaxSampleCount();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

 private:
  VkInstance m_instance;
  VkDebugUtilsMessengerEXT m_debugMessenger;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  GlorpWindow &m_window;
  VkCommandPool m_commandPool;

  VkDevice m_device_;
  VkSurfaceKHR m_surface_;
  VkQueue m_graphicsQueue_;
  VkQueue m_presentQueue_;

  VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

  const std::vector<const char *> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
  #ifdef APPLE
  const std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,"VK_KHR_portability_subset"};
  #else
  const std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  #endif
};

}
