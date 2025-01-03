# Matrices

A matrix $A \in Mat(m, n)$ is an ordered table of numbers sorted in $m$ rows and $n$ columns. $a_{ij}$ indicates the element located in the $i$-th row and $j$-th column.

## Definitions

### Operations

- **Scalar product:** $(\alpha A)_{ij} = \alpha \cdot a_{ij} \quad \alpha \in \C$ 
- **Matrix sum:** $(A + B)_{ij} = (a_{ij} + b_{ij})$
- **Matrix product:** $A \cdot B = C \rightarrow c_{ij} = \sum_\limits{k = 1}^p a_{ik} \cdot b_{kj} \quad A \in Mat(m, p), B \in Mat(p, n), C \in Mat(m, n)$

The first two operations are both associative and commutative, while the matrix product is only associative and distributive with respect to the sum, but it is not, in general, commutative.

### Kind of matrices

- **Square matrix:** $A \in Mat(m, m)$

- **Transpose matrix:** $a_{ij} = a^t_{ji} \quad A \in Mat(m, n), A^T \in Mat(n, m)$

- **Transpose conjugate matrix:** $A^\dagger = (A^T)^* \rightarrow a^\dagger_{ij} = \overline{a_{ji}}$

- **Symmetric matrix (o Hermitian):** $A = A^\dagger \Rightarrow \begin{bmatrix}3 & 2 + i\\2-i & 4\end{bmatrix}$

- **Identity matrix:** $IA = AI = A$

- **Diagonal matrix:** $a_{ij} = 0 \quad i \ne j$

- **Lower triangular matrix:** $a_{ij} = 0 \quad i \lt j$

- **Upper triangular matrix:** $a_{ij} = 0 \quad i \gt j$

  

- **Tridiagonal matrix:** $a_{ij} = 0 \quad |i - j| > 1 \Rightarrow \begin{bmatrix}1 & 2 & 0 & 0 \\5 & 6 & 7 & 0\\0 & 10 & 11 & 12\\0 & 0 & 15 & 16\end{bmatrix}$ 

- **Hessemberg matrix:** $a_{ij} = 0 \quad j > i + 1 \text{ or } i \gt j + 1 \Rightarrow \begin{bmatrix}1 & 2 & 3 & 4 \\5 & 6 & 7 & 8\\0 & 10 & 11 & 12\\0 & 0 & 15 & 16\end{bmatrix} \text{ or } \begin{bmatrix}1 & 2 & 0 & 0 \\5 & 6 & 7 & 0\\9 & 10 & 11 & 12\\13 & 14 & 15 & 16\end{bmatrix}$

- **Unit matrix:** $U^\dagger U = UU^\dagger = I \Rightarrow U^\dagger = U^{-1}$

- **Orthogonal matrix:** $U^T U = UU^T = I \Rightarrow U^T = U^{-1}$

- **Normal matrix:** $U^TU = UU^T$

### Matrix inverse

The matrix inverse is defined for square matrices. A square matrix is invertible (i.e. the inverse exists) if and only if its determinant is non-zero. When multiplied with its inverse, the identity matrix will be produced.
$$
\exists A^{-1}: AA^{-1} = I \Longleftrightarrow \det(A) \ne 0
$$


### Matrici definite positive

Data $A \in Mat(m, m)$, $A$ è **definita positiva** se $\forall x \in \R^m, x \ne 0 \qquad \Rightarrow \qquad x^TAx \gt 0$. Le la maggiorazione non è stretta ma è $\ge$, allora $A$ è **semidefinita positiva**. 
Se $x^TAx \lt 0$, allora $A$ è **definita negativa**. Se $x^tAx \le 0$, allora è **semidefinite negativa**.

### Matrici simili (o trasformate per controgradienza)

Una matrice $A$ è **simile** ad una matrice $B$ se $\exists M : A = M^{-1}BM$. Se una matrice è simile ad una matrice **diagonale**, si dice **diagonalizzabile**.
Se $A$ e $B$ sono simili, lo saranno anche $A^s$ e $B^s \quad \forall s \in \N$.
Se due matrici sono simili, lo saranno anche le loro **inverse**, se esistono.

### Matrici diagonalmente dominanti

Una matrice è **diagonalmente dominante** se la **somma del valore assoluto degli elementi** di una riga eccetto quello sulla diagonale **è inferiore al valore assoluto dell'elemento diagonale** sulla stessa riga.
$$
\text{Matrice diagonalmente dominante: } \abs{a_{ii}} \gt \sum_{\substack{j = 1\\j \ne i}}^{m}\abs{aij} \\
\text{Matrice debolmente diagonalmente dominante: } \abs{a_{ii}} \ge \sum_{\substack{j = 1\\j \ne i}}^{m}\abs{aij}
$$

## Determinante

Sia $A \in Mat(m, m)$. Il **determinante** di $A$, indicato con $\det(A)$, è un numero definito dalla **regola di Laplace**. Se una matrice ha determinate pari a $0$ è detta **degenere**.
$$
A_{ji} = \text{Matrice ottenuta cancellando la i-esima riga e j-esima colonna} \\
\text{Complemento algebrico di } a_{ij} = (-1)^{i + j}(\det(A_{ij})) \\
\abs{A} = \det(A) =  \sum_{j = 1}^m a_{ij}\det(A_{iJ})
$$

> **Esempio:**

$$
A = \begin{bmatrix}
1 & 4 & 5 \\
2 & 4 & 1 \\
2 & 1 & 3 \\
\end{bmatrix} \\
\\
\det(A) = (12 - 1) - 4(6 - 2) + 5(2 - 8) = 11 - 16 - 30 = - 35
$$

### Teorema di Binet

$\det(AB) = \det(A) \det(B)$

### Teorema di Sylvester

Sia $A_k$ la sotto matrice formata dalle prime $k$ righe e colonne di $A$, detti anche minori di $A$.

> $A$ è definita positiva $\qquad \Longleftrightarrow \qquad \det(A_k) \gt 0 \quad \forall k \in [1, n]$ 

Se $A$ è definita positiva, se segue che tutti gli elementi sulla diagonale principale sono positivi e che $\abs{a_{ij}} \lt a_{ii}a_{jj}$.

> $A$ è simmetrica, diagonalmente dominante e diagonalmente positiva $\qquad \Rightarrow \qquad A$ è definita positiva

## Matrice inversa

Sia $\hat{A}$ la matrice trasposta dei **complementi algebrici**. Tale matrice gode della proprietà $A\hat{A} = \hat{A}A = \det(A)I_n$. Se $\det(A)\ne 0$, si può anche calcolare la matrice $A^{-1} = \frac{\hat{A}}{\det(A)}: A^{-1}A = AA^{-1} = I_n$. Si noti che i singoli elementi di $A^{-1}$ sono costruiti a partire dai complementi algebrici fratto il determinante di $A$.

Inoltre, per il **teorema di Binet**, si ha che, se $A, B$ sono matrici non degeneri e $C = AB \Rightarrow C^{-1} = B^{-1}A^{-1}$.
Si può verificare facilmente: $CC^{-1} = I \rightarrow ABC^{-1} = I \rightarrow BC^{-1} = A^{-1} \rightarrow C^{-1} = B^{-1}A^{-1}$.
$$
A = \begin{bmatrix}
a_{11} & a_{12} & ... & a_{1n} \\
a_{21} & a_{22} & ... & a_{2n} \\
\vdots & \vdots & \ddots & \vdots \\
a_{n1} & a_{n2} & ... & a_{nn} \\
\end{bmatrix} \\
\\
\hat{A} = \begin{bmatrix}
\det(A_{11}) & \det(A_{12}) & ... & \det(A_{1n}) \\
\det(A_{12}) & \det(A_{22}) & ... & \det(A_{2n}) \\
\vdots & \vdots & \ddots & \vdots \\
\det(A_{n1}) & \det(A_{n2}) & ... & \det(A_{nn}) \\
\end{bmatrix} \\
\\
A^{-1} = \frac{\hat{A}}{\det(A)}
$$

### Unicità della matrice inversa

Supponiamo per assurdo che $\exists B : BA = AB = I_n$. Ne segue che $BAA^{-1} = I_nA^{-1} = B \Rightarrow B = A^{-1}$.

## Prodotto scalare

Siano $a, b \in C^{n\times 1}$. Il **prodotto scalare** $\langle a, b \rangle = a^\dagger b = \sum_\limits{i = 1}^n\bar{a}_ib_i$.

### Proprietà

- $\langle a, a \rangle \ge 0$
- $\langle a, a \rangle = 0 \Leftrightarrow a = 0$
- $\langle a, \alpha b \rangle = \alpha\langle a, b \rangle$
- $\langle \alpha a,  b \rangle = \bar{\alpha}\langle a, b \rangle$
- $\langle a + c,  b \rangle = \langle a, b \rangle + \langle c, b \rangle$
- $\langle a,  b + c \rangle = \langle a, b \rangle + \langle a, c \rangle$
- $\langle b,  a \rangle = \langle \bar{a, b} \rangle = \sum_\limits{i = 1}^n\bar{b}_ia_i$
- $\abs{\langle a,  b \rangle}^2 \le \langle a, a \rangle \langle b, b \rangle$

### Modulo di $a$

Il **modulo** di un vettore $a$ è $\abs{a} = \langle a, a \rangle^{1/2}$. Si applicano tutte le proprietà viste sopra.

> **Esempio:**

$$
a = \begin{bmatrix}
1 & 2 & 3 & 7 & 1
\end{bmatrix} \\
\\
\abs{a} = \langle a, a \rangle^{1/2} =\sqrt{\sum_{i = 1}^5 \bar{a_i}a_i} = \sqrt{1^2 + 2^2 + 3^2 + 7^2 + 1^2} = \sqrt{64} = 8
$$

## Norme vettoriali

Una **norma** è un operatore così definito: $\norm{} : \C^n \rightarrow \R^+$.
La norma è una **funzione continua** delle componenti del vettore. Infatti $\lim_\limits{\delta \rightarrow \infin} \norm{x + \delta} = \norm{x}$.

A partire dalla norma è possibile definire anche la funzione **distanza** come $d(x, y) = \norm{x - y}$.

### Proprietà

- $\norm{x} \ge 0$
- $\norm{x} = 0 \Leftrightarrow x = 0$
- $\norm{\alpha x} = \abs{\alpha}\norm{x}$
- $\norm{x + y} \le \norm{x} + \norm{y}$
- $\norm{x - y} \le \norm{x - z} + \norm{z - y}$
- $\abs{\norm{x} - \norm{y}} \le \norm{x - y} \le \norm{x} + \norm{y}$

### Norma Holderiana

La **norma Holderiana**, anche detta **norma p**, è una funzione che rispetta le proprietà di una funzione **norma** ed è così definita
$$
\norm{x}_p = \left( \sum_{i = 0}^n \abs{x_i}^p \right) ^{\frac{1}{p}} \quad 1 \le p \le \infin
$$
Ci sono 3 valori di p che vengono usati per definire 3 norme particolarmente utillizzate:
$$
\begin{cases}
\norm{x}_1 = \sum_\limits{i = 0}^n \abs{x_i} & p = 1 & \text{ norma 1}\\
\norm{x}_2 = \sqrt{\sum_\limits{i = 0}^n \abs{x_i}^2} & p = 2 & \text{ norma euclidea}\\
\norm{x}_\infin = \max_i\abs{x_i} & p = \infin & \text{ norma infinito o del massimo}\\
\end{cases}
$$
Si può dimostrare che le tre norme sopracitate sono **equivalenti** in $\R^n$. Quindi, $\exists \alpha, \beta \in \R : \alpha |x|_a \le |x|_b \le \beta |x|_a$, dove $a$ e $b$ possono essere due norme distinte tra le sopracitate.

## Norma matriciale

La **norma matriciale** è una funzione così definita: $\norm{} : \C^{n \times n} \rightarrow \R^+$.

### Proprietà

- $\norm{A} \ge 0$
- $\norm{A} = 0 \Leftrightarrow A = 0$
- $\norm{\alpha A} = \abs{\alpha}\norm{A}$
- $\norm{A + B} \le \norm{A} + \norm{B}$
- $\norm{AB} \le \norm{A}\norm{B}$
- $\norm{Ax}_p \le \norm{A} \norm{x}_p$

### Norma indotta

La norma matriciale più comunemente utilizzata è la **norma indotta**. Per comprenderne il significato, si consideri come, in maniera informale, la norma si occupa di misurare la "grandezza" di un vettore. Per le matrici si applica lo stesso principio. Solo che, ricordando che le matrici possono essere viste come applicazioni lineari di funzioni ad un vettore, si misura quanto sono in grado di "allungare" il suddetto vettore. Il massimo "allungamento" che sono in grado di produrre, misurato attraverso la norma del vettore, in rapporto con il vettore originale, viene definito norma indotta della matrice.
$$
A \in \C^{m \times n}, x \in \C^n \\
\\
\norm{A} = \sup_{x \ne 0}\frac{\norm{Ax}}{\norm{x}} = \max_{x \ne 0} \frac{\norm{Ax}}{\norm{x}} = \max_{\norm{x} = 1} \norm{Ax} \\
\\
\begin{cases}
\norm{A}_1 = \max_\limits{1 \le j \le n}\sum_\limits{i = 0}^n \abs{a_{ij}} & \text{ somma per colonna, prende il massimo}\\
\norm{A}_2 = \sqrt{\rho (A^\dagger A)} & \text{ raggio spettrale di A trasposta coniguata per A}\\
\norm{A}_\infin = \max_\limits{1 \le i \le n} \sum_\limits{j = 0}^n \abs{a_{ij}} & \text{ somma per riga, prende il massimo}\\
\end{cases}
$$


## Autovalori e autovettori

Siano $A \in \R^{n \times n}, \lambda \in \C, \overline{x} \in \C^n$. 
$\lambda$ è un autovalore di $A$ e $\overline{x}$ è l'autovettore associato a $\lambda$ se
$$
\exists \ \overline{x} \in \C^n, \overline{x} \ne 0 : (A-\lambda I)\overline{x} = 0
$$
Si indica con $\sigma(A)$ l'insieme degli autovalori di $A$.

Una matrice contiene un autovalore nullo se e solo se è **singolare** ($\det(A)=0$).

Il **raggio spettrale** è l'autovalore più grande in valore assoluto: $\rho = \max_\limits{\lambda \in \sigma(A)} \abs{\lambda}$. 

### Polinomio ed equazione caratteristica

Per conoscere i valori degli autovalori $\lambda_i$ di $A$ è necessario risolvere il **sistema lineare omogeneo**[^1] $(A - \lambda I)x = 0$. Poiché abbiamo imposto che la soluzione non possa essere quella banale ($x = 0$), è necessario che ci siano infinite soluzioni, e che quindi il determinante della matrice dei coefficienti $\det(A - \lambda I) \ne 0$.
Il polinomio $\det(A -\lambda I)$ prende il nome di **polinomio caratteristico**.
L'equazione $\det(A -\lambda I)$ è invece chiamata **equazione caratteristica**.

### Proprietà

- $A$ e $A^T$ hanno gli stessi autovalori. Infatti $\det(A^T - \lambda I) = \det(A - \lambda I)^T$
- $\det(A) = 0 \Leftrightarrow \lambda = 0$
- Se $\det(A) \ne 0 \Rightarrow \exists A^{-1}: \lambda$ autovalore di $A \Rightarrow \lambda^{-1}$ autovalore di $A^{-1}$
  - $Ax = \lambda x \rightarrow A^{-1}Ax = \lambda A^{-1}x \rightarrow \frac{1}{\lambda}x = A^{-1}x$
- $Ax = \lambda x \Rightarrow A^s x = \lambda^s x \quad \forall s \in \N$
- Due matrici **simili** hanno lo stesso polinomio caratteristico e quindi gli stessi autovalori.

### Molteplicità degli autovalori

- **Molteplicità algebrica:** molteplicità del polinomio caratteristico quando scelto come radice
- **Molteplicità geometrica:** numero di autovettori linearmente indipendenti associati ad esso

### Teorema di Gerschgorin

$$
A \in \C^{n \times n}, i \in [0, n] \\
\\
\rho_i = \sum_\limits{\substack{j = 1\\ j \ne i}}^n \abs{a_{ij}} \\ 
\gamma_i = \{z \in \C : \abs{z - a_{ii}} \le p_i \} \\
\gamma = \bigcup_{i = 1}^n\gamma_i \\
\\
\lambda \in \sigma(A) \Rightarrow \lambda \in \gamma
$$

Il teorema di Gerschgorin afferma che esistono $n$ dischi sul piano complesso, che hanno come centro ognuno dei valori sulla diagonale principale della matrice, Il raggio del disco viene calcolato come somma dei valori assoluti degli elementi non sulla diagonale principale.
$$
Ax = \lambda x \\
\text{Per ogni riga } i \\
\sum_{j = 1} a_{ij}x_j = a_{ii}x_i + \sum_{\substack{j = 1 \\ j \ne i}} a_{ij}x_j = \lambda x_i \\
x_r = \norm{x} \\
a_{rr}x_r + \sum_{\substack{j = 1 \\ j \ne r}} a_{rj}x_j = \lambda x_r \\
(a_{rr} - \lambda)x_r = - \sum_{\substack{j = 1 \\ j \ne r}} a_{rj}x_j \\
\abs{(a_{rr} - \lambda)x_r)} = \abs{\sum_{\substack{j = 1 \\ j \ne r}} a_{rj}x_j} \\
\abs{a_{rr} - \lambda} = \abs{\sum_{\substack{j = 1 \\ j \ne r}} a_{rj}\frac{x_j}{x_r}} \le \sum_{\substack{j = 1 \\ j \ne r}} \abs{a_{rj}}\abs{\frac{x_j}{x_r}} \le \sum_{\substack{j = 1 \\ j \ne r}} \abs{a_{rj}}
$$
Poiché ogni matrice e la sua trasposta hanno gli stessi autovalori, e questi sono contenuti solo e soltanto negli insiemi di valori determinati rispettivamente da $\gamma$ e $\gamma'$, ne segue che $\sigma(A) = \sigma(A^T) \in \gamma \bigcap \gamma'$ .

Dal teorema di Garschgorin emerge anche il fatto che u**na matrice diagonalmente dominante non può avere determinante pari a 0**.

Ricordando infatti la definizione di matrice diagonalmente dominante e combinandola con il teorema di Garshgorin si ottiene che
$$
A \in \C^{n \times n} \\
\text{Diagonalmente dominante: } a_{ii} \gt \sum_{\substack{j = 1\\j \ne i}}^n \abs{a_{ij}} \quad \forall i \in [0, n] \\
\det(A) = 0 \Rightarrow \lambda = 0 \\
\text{Teorema di Gashgorin: } \exists r \in \N : \abs{a_{rr}} \le \sum_{\substack{j = 1\\j \ne i}}^n \abs{a_{ij}} \\
\bot
$$

## Ulteriori teoremi e proprietà

- Una matrice $A \in \C^{n \times n}$ è **diagonalizzabile** se e solo se ha $n$ **autovettori linearmente indipendenti**
- Se $A$ è **hermitiana allora è diagonalizzabile**
- **Teorema di Schur:** $A \in \C^{n \times n} \Rightarrow \exists U \text{ unitaria} : T = U^\dagger A U$, dove $T$ è una matrice triangolare superiore. Se $A \in \R^{n \times n}$, allora $U$ è ortogonale.
- Una matrice tale che $\lim_\limits{s \rightarrow \infin} A^s = 0$ si dice **convergente**