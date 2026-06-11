# Geometry Modeling — TP1 (Mesh Viewer)

Projet du cours **Geometry Modeling** (ESIEE).  
Application C++ qui charge un maillage OBJ, le manipule avec une structure **half-edge**, et affiche le résultat avec OpenGL.

Développé et testé sur **macOS** (Homebrew : GLEW, GLM, GLUT).

---

## État des travaux

| Travail obligatoire | Statut |
|---|---|
| `readFile` | Fait |
| `computeNormals` | Fait |
| Silhouette | Fait |
| Triangulation (faces convexes) | Fait |
| Triangulation (faces concaves) | Fait |
| Triangulation (polygones avec trous) | Non fait (option expert) |
| Tests half-edge | Fait |
| Surface de révolution | Fait |
| Simplification de maillage (edge collapse + QEM) | Fait |
| Subdivision Catmull-Clark | Fait |

---

## Structure du programme

```
TP1/Linux-macOS/MeshViewerCMake/
├── main.cpp              → interface GLUT, affichage, menus, silhouette
├── myMesh.cpp / .h       → cœur du TP (lecture, algorithmes, tests)
├── myVertex.cpp          → sommets + normale au sommet
├── myFace.cpp            → faces + normale à la face
├── myHalfedge.cpp        → demi-arêtes (twin, next, prev, source, face)
├── helperFunctions.h     → buffers OpenGL, caméra, shaders
├── tests/                → tests modulaires (ex. Catmull-Clark)
└── shaders/              → light.vert.glsl, light.frag.glsl
```

**Principe général :** `readFile` construit le maillage half-edge → les algorithmes modifient `vertices / halfedges / faces` → `makeBuffers` envoie les données à OpenGL pour l’affichage.

---

## Détail de ce qui a été fait

### 1. `readFile`

On lit un fichier OBJ (`v` pour les sommets, `f` pour les faces) et on construit la structure half-edge en reliant `next`, `prev`, `source`, `adjacent_face` et les `twin` via une `std::map`, comme dans le squelette du TP.

---

### 2. `computeNormals`

Pour chaque face, la normale est obtenue par produit vectoriel de deux arêtes du triangle ; pour chaque sommet, on parcourt les faces voisines avec `originof` puis `twin->next`, on somme leurs normales, puis on divise et on normalise, comme vu en TD sur la moyenne des normales de faces autour d’un sommet.

---

### 3. Silhouette

Pour chaque arête intérieure, on calcule le milieu, le vecteur caméra → milieu, puis les produits scalaires avec les normales des deux faces voisines, si les signes sont opposés (`d1 * d2 < 0`), l’arête appartient à la silhouette (discontinuité de vue).

---

### 4. Triangulation

On parcourt le contour d’une face avec les half-edges (`next`), puis on applique l’**ear-clipping** : pour les faces concaves, on projette d’abord le polygone en 2D en choisissant l’axe le plus stable selon la normale de la face, avant de reconstruire les triangles dans la structure half-edge.

---

### 5. Simplification (`simplify`)

**Idée du cours :** *edge collapse* — fusionner deux sommets d’une arête, supprimer les faces dégénérées, mettre à jour la connectivité ; en version avancée, **QEM (Quadric Error Metrics)** pour choisir l’arête et la position optimale via les quadrics des plans voisins.

**Ce qu’on a fait :** calcul des quadrics par sommet, coût `vᵀQv` pour chaque arête, sélection de l’arête au coût minimal, collapse avec la position optimale (ou milieu / sommets en fallback), puis reconstruction du maillage.

Je n’ai pas l’impression d’avoir tout maîtrisé à fond cette partie, mais j’ai essayé de suivre la méthode du cours et d’obtenir un résultat visible sur le dauphin (`dolphin.obj`) qui sont pour moi pas ceux qui étaient attendu.

---

### 6. Surface de révolution (`generateSurface`)

On définit un profil 2D `(rayon, hauteur)`, on le fait tourner autour de l’axe Y avec `cos` / `sin` pour créer des anneaux de sommets, puis on relie les anneaux en quads et on construit la connectivité half-edge (même logique que `readFile`).  
Fichier de test : `surface_revolution.obj`

---

### 7. Subdivision Catmull-Clark (`subdivisionCatmullClark`)

**Idée du cours :** subdiviser un maillage en quads en calculant les points de face, d’arête et de sommet (formules Catmull-Clark), puis reconstruire le maillage en quads avec les twins.

**Ce qu’on a fait :** implémentation sur un cube en quads, après plusieurs subdivisions, la forme tend vers une sphère. Captures dans `TP1/Photos TP1/` (`Catmull Clark 0.png` à `4.png`).

---

### 8. Tests half-edge

Comme demandé en  par vous meme, voici les tests modulaires mis en place (un test = une responsabilité) :

- `testTwins()` — chaque demi-arête a un twin (ou bord)
- `testNext()` — cohérence `next` / `prev`
- `testFaces()` — une face pointe vers une demi-arête valide
- `testVertices()` — un sommet pointe vers une demi-arête dont il est la source

`checkMesh()` appelle les quatre tests. Un test séparé `CatmullClarkTest` vérifie aussi la subdivision sur le cube.

---

## Bibliothèques

| Outil | Rôle |
|---|---|
| **C++** | langage du TP |
| **CMake** | compilation |
| **OpenGL + GLEW + GLUT** | rendu et interface |
| **GLM** | matrices (vue, projection) |

---

## Fichiers OBJ de test

Certains modèles ont été générés ou exportés avec **[Meshy AI](https://www.meshy.ai/)** (export OBJ).  
D’autres viennent de `TP1/misc/Models/` (dolphin, kelvin_cell, gear, etc.).

---

## Captures / preuves

Screenshots du TP : `TP1/Photos TP1/`

---

## Aide extérieure et IA (transparence)

### Configuration macOS
- **Codex (Cursor)** a aidé à configurer l’environnement de développement (CMake, GLEW, GLUT, chemins Homebrew sur Mac).
- **ChatGPT** n’a pas réussi à résoudre correctement la configuration sur macOS.

### Triangulation concave
- Aide IA pour l’**ear-clipping** et la **projection 3D → 2D** (choix de l’axe selon la normale de la face).
- L’algorithme vient du cours : je me suis imprégné du cours pour coder, avec une aide technique sur la partie implémentation.

### Simplification
- Aide IA pour la **simplification** (edge collapse et QEM).

### Catmull-Clark
- La **subdivision Catmull-Clark** a été codée à partir du cours, à la main dans `myMesh.cpp`.
- **Gemini** (pas ChatGPT) m’a aidé ensuite sur le reste : intégration dans le viewer (`main.cpp`, `helperFunctions.h`), fichier de test `tests/catmull_clark_test.cpp`, corrections d’affichage / buffers OpenGL et problèmes de compilation — parce qu’une fois l’algo dans `myMesh.cpp`, j’avais du mal à faire tourner le programme correctement à l’écran. (En gros lorsque j'appuyais sur Catmull Subidvsion ca ne marchait qu'une fois sur 2)

### Syntaxe C++
Pour `std::map`, `std::array`, lambdas dans `triangulate`, aide ponctuelle sur la syntaxe C++.
