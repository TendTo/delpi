$$
\boldsymbol{U}^{(1)} = \begin{bmatrix}
  \boldsymbol{U_{1\dots r - 1}} & \boldsymbol{U}^{(1)}_r & \boldsymbol{U_{r + 1\dots m}}
\end{bmatrix} = \newline
= \begin{bmatrix}
  u_{1,1} & u_{2,2} & \dots & u_{1,r-1} & u^{(1)}_{1,r} & u_{1,r+1} & \dots & u_{1,m-1} & u_{1,m} \newline
   		   & u_{2,2} & \dots & u_{2,r-1} & u^{(1)}_{2,r} & u_{2,r+1} & \dots & u_{2,m-1} & u_{2,m} \newline
  && \ddots && \vdots &&&& \vdots \newline
    & & & u_{r - 1,r - 1} & u_{r - 1,r} & u_{r-1,r+1} & \dots & u_{r-1,m-1} & u_{r-1,m} \newline
  & & &  & u_{r,r} & u_{r,r+1} & \dots & u_{r,m-1} & u_{r,m} \newline
  & & & & u_{r+1,r} & u_{r+1,r+1} & \dots & u_{r+1,m-1} & u_{r+1,m} \newline
  &&&& \vdots & \ddots &&& \vdots \newline
  & & && u_{m - 1,r} &&& u_{m-1,m-1} & u_{m-1,m} \newline
  & & & & u_{m,r} &&& u_{m,m-1} & u_{m,m}
\end{bmatrix} \quad
$$

$$
\boldsymbol{w} = [\boldsymbol{M}^{(k)}_{m-1}\boldsymbol{\Pi}^{(k)}_{m-1} \dots \boldsymbol{M}^{(k)}_{r}\boldsymbol{\Pi}^{(k)}_{r} \boldsymbol{H}^{(k)}] \dots [\boldsymbol{M}^{(1)}_{m-1}\boldsymbol{\Pi}^{(1)}_{m-1} \dots \boldsymbol{M}^{(1)}_{r}\boldsymbol{\Pi}^{(1)}_{r} \boldsymbol{H}^{(1)}] \boldsymbol{t}
$$

