;; -*- lexical-binding: t; -*-

(TeX-add-style-hook
 "prova"
 (lambda ()
   (TeX-add-to-alist 'LaTeX-provided-class-options
                     '(("article" "")))
   (TeX-add-to-alist 'LaTeX-provided-package-options
                     '(("babel" "italian") ("listings" "") ("xcolor" "") ("geometry" "letterpaper" "top=2cm" "bottom=2cm" "left=3cm" "right=3cm" "marginparwidth=1.75cm") ("amsmath" "") ("graphicx" "") ("hyperref" "colorlinks=true" "allcolors=blue")))
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "href")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "hyperimage")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "hyperbaseurl")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "nolinkurl")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "url")
   (add-to-list 'LaTeX-verbatim-macros-with-braces-local "path")
   (add-to-list 'LaTeX-verbatim-macros-with-delims-local "path")
   (TeX-run-style-hooks
    "latex2e"
    "article"
    "art10"
    "babel"
    "listings"
    "xcolor"
    "geometry"
    "amsmath"
    "graphicx"
    "hyperref"))
 :latex)

