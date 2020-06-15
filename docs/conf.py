from datetime import datetime
from pathlib import Path
import os
import re

SPINNER_BASE = os.environ["SPINNER_BASE"]
DOXYGEN_OUTPUT_DIRECTORY = os.environ["DOXYGEN_OUTPUT_DIRECTORY"]

# Project ----------------------------------------------------------------------

project = "Spinner"
author = "Teslabs Engineering S.L."
year = 2021
copyright = f"{year}, {author}"

cmake = Path(SPINNER_BASE) / "spinner" / "CMakeLists.txt"
with open(cmake) as f:
    version = re.search(r"project\(.*VERSION\s+([0-9\.]+).*\)", f.read()).group(1)

# Options ----------------------------------------------------------------------

extensions = [
    "sphinx.ext.intersphinx",
    "breathe",
    "sphinx.ext.mathjax",
    "sphinxcontrib.bibtex",
    "matplotlib.sphinxext.plot_directive",
]

numfig = True

# HTML output ------------------------------------------------------------------

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "logo_only": True,
    "collapse_navigation": False,
}
html_static_path = ["_static"]
html_logo = "_static/images/logo.svg"

# Options for intersphinx ------------------------------------------------------

intersphinx_mapping = {"zephyr": ("https://docs.zephyrproject.org/latest", None)}

# Options for breathe ----------------------------------------------------------

breathe_projects = {"app": str(Path(DOXYGEN_OUTPUT_DIRECTORY) / "xml")}
breathe_default_project = "app"
breathe_domain_by_extension = {"h": "c", "c": "c"}
breathe_default_members = ("members", )

# Options for sphinxcontrib.bibtex ---------------------------------------------

bibtex_bibfiles = ["bibliography.bib"]


def setup(app):
    app.add_css_file("css/custom.css")
    app.add_css_file("css/light.css")
