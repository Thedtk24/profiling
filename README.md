# Profilage sous Linux

**Sujet :** Améliorer la vitesse de l'application Mandelbrot.
**Outils :** `perf`, `callgrind`, `hotspot`, `kcachegrind`, `hyperfine`.

---

## Travail 0 : Découverte des outils

Nous avons comparé deux programmes "Hello World", l'un en C, l'autre en C++.

*   **Différence :** Même si le résultat affiché est le même, le programme en C++ fait plus de choses en coulisses (comme préparer des librairies complexes).
*   **Problème rencontré avec `perf` :** Si un programme va trop vite (moins de 1ms), `perf` n'a pas le temps de voir ce qu'il se passe (fichier vide). Dans ce cas, on utilise `Callgrind` qui compte chaque instruction une par une, ce qui est beaucoup plus précis.
Les resultats sont disponibles dans le dossier `reports`.

---

## Travail 1 : Les différentes versions du code

Nous avons testé plusieurs modifications pour voir laquelle est la plus rapide :

*   **`master` (Original)** : La version de base. Elle utilise des nombres très précis (`double`) et une fonction standard pour calculer la distance (`cabs`).
*   **`inline`** : On a réécrit le calcul de distance directement dans le code pour éviter de perdre du temps à appeler une fonction externe.
*   **`nosqrt` (Sans racine carrée)** : Astuce mathématique. Au lieu de calculer la racine carrée (ce qui est lent), on compare le carré des nombres. C'est beaucoup plus rapide et donne le même résultat visuel.
*   **`fp32` (Nombres simples)** : On utilise des nombres moins précis mais plus rapides (`float` au lieu de `double`) pour que le processeur puisse en calculer plusieurs à la fois (**vectorisation SIMD**).
*   **`zoom`** : On change la zone de l'image calculée pour augmenter la difficulté et mieux voir les différences.

---

## Travail 2 : Mesures de vitesse (Hyperfine)

Nous avons chronométré chaque version avec `hyperfine`. Voici ce qu'on a découvert :

### Résultats

| Version | Temps moyen | Vitesse comparée | Pourquoi ? |
| :--- | :--- | :--- | :--- |
| `master` | 4.22 s | Référence | Utilise une fonction lente (`cabs`). |
| `fp32` | 9.49 s | 2x plus LENT ! | Paradoxe expliqué ci-dessous. |
| `inline` | 1.44 s | ~3x plus rapide | On évite les appels de fonction inutiles. |
| `nosqrt` | 1.50 s | ~2.8x plus rapide | On évite la racine carrée coûteuse. |
| `fp32_nosqrt` | **1.38 s** | **3x plus rapide** | La meilleure version. |

### Le mystère du `fp32` (Pourquoi c'est plus lent ?)
La version `fp32` seule est lente car elle utilise une fonction standard (`cabsf`) qui fait beaucoup de vérifications de sécurité inutiles ici (gestion des erreurs, infinis...).
Par contre, la version **`fp32_nosqrt`** est la plus rapide car on a supprimé cette fonction lente. Le compilateur est alors libre d'utiliser des instructions spéciales (AVX/SSE) pour calculer plusieurs pixels en même temps sans être interrompu.

---

## Travail 3 : Analyse approfondie (Pourquoi on gagne du temps ?)

### 1. Avec `perf` (Hotspot)
Au début (`master`), `perf` nous montre que le processeur passe 60% de son temps coincé dans la librairie mathématique. C'est un goulot d'étranglement.
En utilisant l'**inlining** (réécrire la fonction soi-même), ce blocage disparaît. Le processeur passe alors tout son temps à faire le calcul pur.

### 2. Avec `Callgrind`
Cet outil nous confirme que l'astuce mathématique **`nosqrt`** réduit le nombre d'étapes de calcul de 15%.
De plus, utiliser des nombres simples (`float`) réduit de moitié la quantité de données à écrire en mémoire, ce qui soulage les échanges de données.

---

## Travail 4 : Utiliser tous les cœurs (OpenMP)

Chaque pixel de l'image est calculé indépendamment des autres. On peut donc facilement distribuer le travail sur plusieurs cerveaux (cœurs) du processeur.

*   **La Stratégie :** On utilise le mode `dynamic`.
*   **Pourquoi ?** Certaines zones de l'image (le noir) sont très longues à calculer, d'autres (le blanc) très rapides. Si on coupe l'image en parts égales, certains cœurs finiraient trop vite et attendraient les autres. Le mode `dynamic` distribue les tâches intelligemment : dès qu'un cœur a fini, il en prend une nouvelle (c'est l'équilibrage de charge).
*   **Gain :** Le temps d'exécution de chaque version du programme baisse drastiquement'.

---

## Ce qu'il faut retenir

L'analyse nous a appris trois leçons importantes :
1.  **Réécrire les petites fonctions (Inlining)** est utile si la librairie standard est trop prudente et lente.
2.  **Simplifier les maths (NoSqrt)** donne un gain de vitesse immédiat en supprimant des calculs lourds.
3.  **Utiliser des nombres simples (`float`)** n'est utile que si rien ne bloque le compilateur (comme des appels de fonction externes) pour qu'il puisse vectoriser.

**La recette gagnante :**
1. Mesurer le temps global avec `hyperfine`.
2. Trouver précisément où ça bloque avec `perf`.
3. Corriger le code.
4. Utiliser tous les cœurs avec `OpenMP`.
