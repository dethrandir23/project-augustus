cmake_minimum_required(VERSION 3.15)

# Proje Adı ve Dili
project(producer LANGUAGES CXX)

# C++ Standardı (Senin kod C++20 istiyor)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define: UUID kütüphanesi için gerekli
#add_compile_definitions(LIBUUID_CPP20_OR_GREATER)

# Include Dizinleri
# src ve lib klasörlerini ekliyoruz ki #include yaparken rahat et.
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib
)

# Kaynak Dosyaları Bul (Senin tree yapına göre recursive tarar)
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Executable Oluştur
add_executable(producer ${SOURCES})

# --------------------------------------------------------
# EMSCRIPTEN AYARLARI (WebAssembly Büyüsü Burada)
# --------------------------------------------------------
if(EMSCRIPTEN)
    # Çıktı uzantısı .js olacak (yanında .wasm da üretecek)
    set_target_properties(producer PROPERTIES SUFFIX ".js")

    # Linker Flagleri (Senin uzun komuttaki her şey burada)
    target_link_options(producer PRIVATE
        "--bind"                        # Embind (GameApi.cpp için şart)
        "-sWASM=1"                      # WebAssembly çıktısı
        "-sALLOW_MEMORY_GROWTH=1"       # RAM yetmezse tarayıcıdan ister
        "-sMODULARIZE=1"                # JS tarafında modül olarak kullanmak için
        "-sEXPORT_NAME='GameModule'"    # JS'de: var Module = GameModule();
        "-sNO_EXIT_RUNTIME=1"           # main bitince programı kapatma (step() çağıracaz)
        "-sASSERTIONS=1"                # Debug için (Hata varsa konsola basar)
        # "-O3"                         # Release alırken bunu açarsın, dosya boyutu küçülür
    )
    
    # Derleme seçenekleri
    target_compile_options(producer PRIVATE 
        "-fexceptions"                  # Exception handling gerekebilir
    )
    
    message(STATUS "Emscripten build detected. Output will be game.js and game.wasm")
else()
    message(STATUS "Native build detected.")
endif()