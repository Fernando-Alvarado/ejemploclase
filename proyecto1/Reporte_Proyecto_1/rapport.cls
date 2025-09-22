\LoadClass[a4paper,12pt]{article} % Clase del documento

%--------------------- Paquetes ------------------------

\RequirePackage[spanish]{babel} % Idioma del documento
\RequirePackage[utf8]{inputenc}
\RequirePackage[section]{placeins}
\RequirePackage[T1]{fontenc}
\RequirePackage{mathtools}
\RequirePackage{siunitx}
\RequirePackage{float}
\RequirePackage{graphicx}
\RequirePackage[justification=centering]{caption}
\RequirePackage{subcaption}
\RequirePackage{wallpaper}
\RequirePackage{nomencl}
\RequirePackage{fancyhdr}
\RequirePackage{url}
\RequirePackage[hidelinks]{hyperref}
\RequirePackage[left=2.5cm,right=2.5cm,top=2cm,bottom=3.5cm]{geometry}

%-------------------- Información del reporte ----------------------

\newcommand{\subject}[1]{\renewcommand{\subject}{#1}}
\newcommand{\topic}[1]{\renewcommand{\topic}{#1}}
\newcommand{\ttitle}[1]{\renewcommand{\ttitle}{#1}}
\newcommand{\professor}[1]{\renewcommand{\professor}{#1}}
\newcommand{\students}[1]{\renewcommand{\students}{#1}}
\newcommand{\assistants}[1]{\renewcommand{\assistants}{#1}}
\newcommand{\logo}[1]{\renewcommand{\logo}{#1}}
\newcommand{\uni}[1]{\renewcommand{\uni}{#1}}

\newcommand{\buildmargins}{
\makenomenclature
\pagestyle{fancy}
\fancyheadoffset{1cm}
\setlength{\headheight}{2cm}
\lhead{\includegraphics[scale=0.6]{logos/Logo-UNAM-Azul-Landscape.png}}
\rhead{\nouppercase{\leftmark}}
\rfoot{\thepage}
\cfoot{} 
\lfoot{\subject}
}

\newcommand{\buildcover}{
\begin{titlepage}

    % Logos en las esquinas superiores
    \begin{minipage}{0.49\textwidth}
        \flushleft
        \includegraphics[width=0.67\textwidth]{logos/UNAM.png} % Logo superior izquierdo
    \end{minipage}
    \begin{minipage}{0.49\textwidth}
        \flushright
        \includegraphics[width=0.6\textwidth]{logos/facultad-de-ciencias.jpg} % Logo superior derecho
    \end{minipage}
    
    \centering
    \vspace{1cm} % Ajusta el espacio entre los logos y el logo central
    \includegraphics[width=0.3\textwidth]{\logo}\par\vspace{1cm} % Logo principal
    {\scshape\LARGE \uni \par} % Nombre de la universidad
    \vspace{1.5cm}
    \rule{\linewidth}{0.2 mm} \\[0.4 cm]
    {\huge\bfseries \ttitle \par}
    \rule{\linewidth}{0.2 mm} \\[1.5 cm]
    \vspace{1cm}
    
    % Sección de estudiantes y profesor
    \begin{minipage}{0.5\textwidth}
        \begin{flushleft} \large
        \emph{\textbf{Estudiante :}}\\
        \students\\ % Lista de estudiantes
        \end{flushleft}
    \end{minipage}
    ~
    \begin{minipage}{0.4\textwidth}
        \begin{flushright} \large
        \emph{\textbf{Profesor :}} \\
         \professor \\ % Nombre del profesor
         \vspace{0.5cm} % Espacio entre Profesor y Ayudantes
         \emph{\textbf{Ayudante :}} \\
         \assistants \\ % Nombres de los ayudantes
        \end{flushright}
    \end{minipage}\\[4cm]

\end{titlepage}
}

\newcommand{\toc}{
\tableofcontents
\newpage
}

\newcommand{\insererfigure}[4]{
\begin{figure}[ht]
\centering
\includegraphics[height=#2]{#1}
\caption{#3}
\label{fig: #4}
\end{figure}
}
