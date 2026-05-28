// TerrainGenerator.h
#include <FastNoise/FastNoise.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

class TerrainGenerator {
public:
  TerrainGenerator() : size_x(4000), size_z(4000) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    seed = dis(gen);
    init();
  }

  TerrainGenerator(size_t x, size_t z) : size_x(x), size_z(z) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    seed = dis(gen);
    init();
  }

  TerrainGenerator(size_t x, size_t z, int s) : size_x(x), size_z(z), seed(s) {
    init();
  }

  ~TerrainGenerator() {
    if (generateThread && generateThread->joinable()) {
      generateThread->join();
    }
  }

  void init() {
    height.resize(size_x * size_z);
    moisture.resize(size_x * size_z);

    // 1. Hata Fix: OpenSimplex2 yerine Simplex (veya OpenSimplex2S) kullanıyoruz
    auto source = FastNoise::New<FastNoise::Simplex>();
    source->SetSeed(seed);

    auto fractal = FastNoise::New<FastNoise::FractalFBm>();
    fractal->SetSource(source);
    fractal->SetGain(0.5f);
    fractal->SetLacunarity(2.0f);
    fractal->SetOctaveCount(4);
    
    // 2. Hata Fix: SmartNode olduğu için std::move gerekmez, doğrudan atıyoruz
    heightNoise = fractal;

    auto moistSource = FastNoise::New<FastNoise::Simplex>();
    moistSource->SetSeed(seed + 10000);

    auto moistFractal = FastNoise::New<FastNoise::FractalFBm>();
    moistFractal->SetSource(moistSource);
    moistFractal->SetOctaveCount(2);
    moistFractal->SetGain(0.4f);
    moistureNoise = moistFractal;
  }

  void generateTerrainSync() {
    if (size_x == 0 || size_z == 0)
      return;

    generating.store(true);
    completed.store(false);
    error.store(false);

    for (int y = 0; y < (int)size_z; ++y) {
      for (int x = 0; x < (int)size_x; ++x) {
        float fx = x * 0.005f;
        float fy = y * 0.005f;

        // 3. Hata Fix: GenSingle2D artık 3. parametre olarak seed istiyor
        float h = heightNoise->GenSingle2D(fx, fy, seed);
        float m = moistureNoise->GenSingle2D(fx * 1.3f, fy * 1.3f, seed + 10000);

        float heightNorm = (h + 1.0f) / 2.0f;
        float moistureAdjusted =
            m - heightNorm * 0.5f;
        moistureAdjusted = std::clamp(moistureAdjusted, -1.0f, 1.0f);

        height[y * size_x + x] = heightNorm * 255.0f;
        moisture[y * *size_x + x] = (moistureAdjusted + 1.0f) * 127.5f;
      }
    }

    generating.store(false);
    completed.store(true);
  }

  void generateTerrainAsync() {
    if (generateThread && generateThread->joinable()) {
      generateThread->join();
    }

    generateThread =
        std::make_unique<std::thread>([this]() { generateTerrainSync(); });
  }

  void waitForCompletion() {
    if (generateThread && generateThread->joinable()) {
      generateThread->join();
    }
  }

  bool isGenerating() const { return generating.load(); }
  bool isCompleted() const { return completed.load(); }

  // Erişim fonksiyonları
  uint8_t getHeight(size_t x, size_t z) const {
    if (x >= size_x || z >= size_z)
      return 0;
    return height[z * size_x + x];
  }

  uint8_t getMoisture(size_t x, size_t z) const {
    if (x >= size_x || z >= size_z)
      return 0;
    return moisture[z * size_x + x];
  }

  void getHeightAndMoisture(size_t x, size_t z, uint8_t &outHeight,
                            uint8_t &outMoisture) const {
    size_t idx = z * size_x + x;
    if (x < size_x && z < size_z) {
      outHeight = height[idx];
      outMoisture = moisture[idx];
    }
  }

  int calcBiomeType(float heightVal, float moistureVal) {
    // Girdiler 0-255 aralığında, 0-1'e normalize et
    float h = heightVal / 255.0f;
    float m = moistureVal / 205.0f; // Normalize 0-1

    if (h < 0.3f) {
      return 0; // Okyanus / Deniz / Göl
    } else if (h < 0.45f) {
      if (m > 0.6f) {
        return 1; // Plaj / Sulak Alan
      } else {
        return 2; // Kurak Arazi / Çöl başlangıcı
      }
    } else if (h < 0.7f) {
      if (m > 0.6f) {
        return 3; // Orman
      } else if (m > 0.3f) {
        return 4; // Ova / Mera
      } else {
        return 5; // Step / Savan
      }
    } else if (h < 0.9f) {
      return 6; // Dağ etekleri / Kayalık arazi
    } else {
      return 7; // Karlı zirveler
    }
  }

  int getBiomeType(size_t x, size_t z) {
    if (x >= size_x || z >= size_z)
      return -1;
    return calcBiomeType(height[z * size_x + x], moisture[z * size_x + x]);
  }

  size_t getSizeX() const { return size_x; }
  size_t getSizeZ() const { return size_z; }
  int getSeed() const { return seed; }

  const uint8_t *getHeightData() const { return height.data(); }
  const uint8_t *getMoistureData() const { return moisture.data(); }

private:
  int seed = 0;
  size_t size_x = 4000;
  size_t size_z = 4000;

  std::atomic<bool> completed{false};
  std::atomic<bool> generating{false};
  std::atomic<bool> error{false};

  std::vector<uint8_t> height;
  std::vector<uint8_t> moisture;

  // 2. Hata Fix: SmartNode kullanarak FastNoise2 mimarisine uyumlu hale getirdik
  FastNoise::SmartNode<const FastNoise::Generator> heightNoise;
  FastNoise::SmartNode<const FastNoise::Generator> moistureNoise;

  std::unique_ptr<std::thread> generateThread;
};
