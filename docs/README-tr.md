# PROJECT AUGUSTUS
🌐 [English](../README.md) | [Türkçe](README-tr.md)

---

## Oyun Konsepti

Bu oyunda bir şirketi yönetiyorsun. İstersen senaryoda hazır bulunan şirketlerden birini seçebilir, istersen sıfırdan kendi şirketini kurabilirsin. Başlangıç için farklı template seçenekleri mevcut: büyük ama borç içinde bir şirket, küçük ve az sermayeli klasik bir başlangıç veya farklı risk profillerine sahip alternatif senaryolar.

---

## Dünya ve Ekonomi

Oyun dünyasında **TradeNode** adı verilen yerleşkeler bulunur. Bunlar köyler, şehirler, limanlar veya çöl kasabaları gibi farklı tiplerde olabilir.

Her TradeNode'un kendine özgü:

* Nüfusu
* Üretimi
* Tüketimi
* Kaynak yapısı

vardır. Template'lere göre değişiklik gösterirler. Örneğin bir köy ile büyük bir liman şehrinin üretim ve tüketim dengesi aynı değildir.

Eğer bir yerleşke 3 birim buğday üretiyor ama nüfusunun 6 birim buğday tüketmesi gerekiyorsa, aradaki açık pazarda **talep (demand)** olarak oluşur. Aynı şekilde, tüketmediği kaynakları (örneğin demir cevheri) satmak ister.

---

## Pazar ve Orderbook Sistemi

Her TradeNode bir pazara bağlıdır. Pazarlar, mallar için ayrı ayrı **orderbook** sistemleri tutar.

* Alım-satım işlemleri dinamik olarak gerçekleşir
* Fiyatlar son gerçekleşen takasa göre güncellenir
* Arz ve talep ekonomiyi doğrudan şekillendirir

Eğer bir tur boyunca bir yerleşkenin tüketici ihtiyaçları yeterince karşılanırsa:

* Nüfus artar
* Mutluluk artar
* Büyüme oranı bonus kazanır

Karşılanmazsa:

* Nüfus azalır
* Ekonomik daralma yaşanabilir

---

## Şirket Yönetimi ve Üretim

Şirket olarak fabrikalar kurarsın. Ancak üretim yapabilmek için:

* İşçi sağlamalısın
* Hammadde tedarik etmelisin
* Lojistiği kurmalısın

Ürettiğin malları en kârlı pazarlarda satmak senin sorumluluğundadır.

Karlılığı artırmak için:

* Piyasa analizi yapabilir
* Piyasa manipülasyonu uygulayabilir
* Ucuz lojistik hatları kurabilir
* Sürekli ikmal zinciri oluşturabilir
* Kritik olmayan mallarda düşük fiyatlı emir girip hızlı alım yapabilirsin

---

## Lojistik ve Riskler

Lojistik hatların korsan saldırılarına uğrayabilir. Tedarik zinciri kesintiye uğrayabilir.

Risk yönetimi oyunun önemli bir parçasıdır. Kötü planlama tüm üretim zincirini bozabilir.

---

## Borsa ve Finans

Belirli bir sermayeye ulaştığında özel bir event ile halka arz olabilir ve borsaya girebilirsin.

Finansal sistem şunları içerir:

* Diğer şirketlerin hisselerini satın alabilirsin
* Orderbook tabanlı hisse alım-satımı yapabilirsin
* Haberler hisse fiyatlarını etkiler
* Paravan şirketler kurup hisse toplayabilirsin

**İtibar sistemi** mevcuttur. Manipülasyon, sabotaj veya yalan haber gibi eylemler kısa vadeli kazanç sağlasa da uzun vadede itibar kaybı yaratabilir.

---

## Diplomasi ve Devletler

Dünyada devletler vardır ve savaşabilirler.

Savaş zamanında:

* Askeri malların fiyatı artabilir
* Savaşan devletlerin pazarlarına erişimin kısıtlanabilir

Monopol anlaşmaları imzalayabilir, belirli pazarlarda belirli malların tek satıcısı olabilirsin. Ancak bu durum davalara ve hukuki süreçlere yol açabilir.

---

## Hukuk Sistemi

Oyunda hukuk sistemi bulunur:

* İş kazaları
* Çalışma koşulları denetimleri
* Tazminat davaları
* Tekel davaları

Gerekli reformları ve güvenlik önlemlerini uygulamazsan ceza alabilirsin.

---

## Event Sistemi

Oyun dinamik bir event sistemi içerir.

Örnek:

**Depoda yangın çıktı:**

* Çalışanları kurtarmaya öncelik verirsen → daha az personel kaybedersin ama daha fazla mal kaybedersin
* Malları kurtarmaya öncelik verirsen → daha fazla çalışan kaybedebilirsin ancak stok kaybı azalır

İleride çalışanların deneyim puanı olacağı düşünüldüğünde, bu kararlar uzun vadeli stratejik etki yaratır.

---

## Departman ve Advisor Sistemi

Şirket içinde departmanlar bulunur:

* Ticaret
* Borsa
* Lojistik
* İnsan Kaynakları

Her departmana farklı yıldız seviyelerinde AI advisor atayabilirsin.

* Daha yüksek yıldızlı advisorlar daha doğru kararlar verir
* Maaşları daha yüksektir
* İstersen mikroyönetim yapabilir, istersen bazı işlemleri advisorlara devredebilirsin

Örneğin sıradan malları otomatiğe bağlayıp yüksek katma değerli ürünleri kendin yönetebilirsin.

---

## Sonuç

Amaç:

* Daha fazla üretmek
* Daha kârlı satmak
* Daha fazla fabrika kurmak
* Lojistik ağını geliştirmek
* Hisse yatırımları yapmak
* Ekonomik bir imparatorluk kurmak

Ancak yanlış kararlar, kötü eventler, savaşlar, büyük davalar veya senden güçlü rakipler iflasına yol açabilir.

İyi de bitse kötü de bitse, bu senin hikayen.

Günün sonunda kazandığın parayla emekli olabilir ve campaign'i istediğin noktada sonlandırabilirsin.