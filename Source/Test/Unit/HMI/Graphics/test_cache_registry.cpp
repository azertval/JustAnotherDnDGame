// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_cache_registry.cpp
 * @brief Tests unitaires du registre de mémoïsation/invalidation (LOT-43 TACHE-03).
 */

#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "HMI/Graphics/CacheRegistry.h"

namespace {

// Compte les appels au chargeur, pour verifier la memoisation sans dependre du type charge.
class CountingLoader {
public:
    explicit CountingLoader(std::optional<int> result) : _result(result) {}

    std::optional<int> operator()() {
        ++_callCount;
        return _result;
    }

    [[nodiscard]] int callCount() const noexcept {
        return _callCount;
    }

private:
    std::optional<int> _result;
    int _callCount = 0;
};

}  // namespace

/**
 * @brief Une clé demandée deux fois n'est chargée qu'une seule fois, et les deux appels rendent
 * la **même** entrée : c'est toute la raison d'être du registre — un asset affiché à chaque frame
 * ne doit pas relire le disque à chaque frame.
 * \castest{<b>Une clé demandée deux fois n'est chargée qu'une fois et rend la même entrée.</b><br/>
 * \tcat Unitaire · Registre de cache<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(CacheRegistryTest, ChargeUneSeuleFoisPourUneCleRepetee) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loader(42);

    const int* first = registry.getOrLoad("a", [&loader] { return loader(); });
    const int* second = registry.getOrLoad("a", [&loader] { return loader(); });

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(*first, 42);
    EXPECT_EQ(second, first) << "meme entree, pas de rechargement";
    EXPECT_EQ(loader.callCount(), 1);
}

/**
 * @brief `invalidate` ne vise qu'une clé : celle-ci est relue, les autres restent en cache. C'est
 * ce qui rend le rechargement à chaud ciblé — modifier un asset ne doit pas faire relire toute la
 * bibliothèque.
 * \castest{<b>invalidate ne force le rechargement que de la clé visée, pas des autres.</b><br/>
 * \tcat Unitaire · Registre de cache<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(CacheRegistryTest, InvalidateForceLeRechargementDeLaCleSeule) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loaderA(1);
    CountingLoader loaderB(2);

    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));
    registry.invalidate("a");

    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));

    EXPECT_EQ(loaderA.callCount(), 2) << "la cle invalidee doit relire";
    EXPECT_EQ(loaderB.callCount(), 1) << "les autres entrees sont conservees";
}

/**
 * @brief `invalidateAll` vide le registre : sa taille retombe à zéro et **toutes** les clés sont
 * relues au prochain accès. C'est le geste du rechargement global (changement de jeu de skins,
 * bouton « recharger les assets »).
 * \castest{<b>invalidateAll vide le registre et fait relire toutes les clés.</b><br/>
 * \tcat Unitaire · Registre de cache<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(CacheRegistryTest, InvalidateAllViseTout) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loaderA(1);
    CountingLoader loaderB(2);
    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));
    ASSERT_EQ(registry.size(), 2U);

    registry.invalidateAll();

    EXPECT_EQ(registry.size(), 0U);
    static_cast<void>(registry.getOrLoad("a", [&loaderA] { return loaderA(); }));
    static_cast<void>(registry.getOrLoad("b", [&loaderB] { return loaderB(); }));
    EXPECT_EQ(loaderA.callCount(), 2);
    EXPECT_EQ(loaderB.callCount(), 2);
}

/**
 * @brief Un échec de chargement est mémorisé comme tel : la clé absente n'est pas relue à chaque
 * appel. Sans cette mémoire, un asset manquant provoquerait un accès disque infructueux **par
 * frame**, précisément dans le cas où le repli procédural doit rester silencieux et gratuit.
 * \castest{<b>Un échec de chargement est mémorisé : la clé absente n'est pas relue.</b><br/>
 * \tcat Unitaire · Registre de cache<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(CacheRegistryTest, UnEchecEstMemoriseSansRetenterLeChargement) {
    hmi::CacheRegistry<int> registry;
    CountingLoader loader(std::nullopt);

    const int* first = registry.getOrLoad("absent", [&loader] { return loader(); });
    const int* second = registry.getOrLoad("absent", [&loader] { return loader(); });

    EXPECT_EQ(first, nullptr);
    EXPECT_EQ(second, nullptr);
    EXPECT_EQ(loader.callCount(), 1) << "un echec deja constate ne doit pas relire le disque";
}

/**
 * @brief Invalider une clé en échec autorise un nouvel essai, qui peut réussir : c'est le cas du
 * fichier créé **après** une première tentative infructueuse — le rechargement à chaud doit
 * pouvoir rattraper un échec mémorisé, sinon l'asset resterait invisible jusqu'au redémarrage.
 * \castest{<b>Invalider une clé en échec autorise un nouvel essai, qui peut réussir.</b><br/>
 * \tcat Unitaire · Registre de cache<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(CacheRegistryTest, InvalidateSurUnEchecPermetUnNouvelEssai) {
    hmi::CacheRegistry<int> registry;
    CountingLoader failing(std::nullopt);
    static_cast<void>(registry.getOrLoad("asset", [&failing] { return failing(); }));
    registry.invalidate("asset");

    CountingLoader succeeding(7);
    const int* result = registry.getOrLoad("asset", [&succeeding] { return succeeding(); });

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 7);
}
