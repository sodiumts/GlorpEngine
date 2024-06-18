#pragma once
 
#include "glorp_device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace Glorp {
 
class GlorpDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(GlorpDevice &glorpDevice) : glorpDevice{glorpDevice} {}
 
    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<GlorpDescriptorSetLayout> build() const;
 
   private:
    GlorpDevice &glorpDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
 
  GlorpDescriptorSetLayout(
      GlorpDevice &glorpDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~GlorpDescriptorSetLayout();
  GlorpDescriptorSetLayout(const GlorpDescriptorSetLayout &) = delete;
  GlorpDescriptorSetLayout &operator=(const GlorpDescriptorSetLayout &) = delete;
 
  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
 
 private:
  GlorpDevice &glorpDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
 
  friend class GlorpDescriptorWriter;
};
 
class GlorpDescriptorPool {
 public:
  class Builder {
   public:
    Builder(GlorpDevice &glorpDevice) : glorpDevice{glorpDevice} {}
 
    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<GlorpDescriptorPool> build() const;
 
   private:
    GlorpDevice &glorpDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
 
  GlorpDescriptorPool(
      GlorpDevice &glorpDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~GlorpDescriptorPool();
  GlorpDescriptorPool(const GlorpDescriptorPool &) = delete;
  GlorpDescriptorPool &operator=(const GlorpDescriptorPool &) = delete;
 
  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
 
  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
 
  void resetPool();
 
 private:
  GlorpDevice &glorpDevice;
  VkDescriptorPool descriptorPool;
 
  friend class GlorpDescriptorWriter;
};
 
class GlorpDescriptorWriter {
 public:
  GlorpDescriptorWriter(GlorpDescriptorSetLayout &setLayout, GlorpDescriptorPool &pool);
 
  GlorpDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  GlorpDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
 
  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);
 
 private:
  GlorpDescriptorSetLayout &setLayout;
  GlorpDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
 
}