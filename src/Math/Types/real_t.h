// src/Math/Types/real_t.h
#pragma once
#include "udouble.h"
#include "real.h"

// --------------------------------------------------------------------------
// Merkezi Tip Tanımlamaları
// Projedeki tüm ekonomi, nüfus ve simülasyon mantığı bu tipleri kullanmalıdır.
// Bu sayede gelecekteki mimari değişiklikleri tek satırda çözülebilir.
// --------------------------------------------------------------------------

// Eğer tam güvenlik ve NaN koruması istiyorsan wrapper olan `real` sınıfını,
// ham performans ve raw c++ türü istiyorsan `double` tipini seçebilirsin.
// Şimdilik motorun standartını direkt native double olarak ayarlıyoruz:
using real_t = double; 

// Negatif olamayan kaynaklar (nüfus, item miktarı) için:
using ureal_t = udouble;
