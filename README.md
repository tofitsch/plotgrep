Work in progress!

# Intro
## Synopsis
CLI tool for indexing both plots and text in input PDFs (slides/papers) and then to search in them.

Plots are stored as hex-encoded code from bitmaps containing the most-relevant components of a discrete cosine transform of the original plot.
Hence, similar looking plots get assigned similar codes.
The search of a given plot in the index is performed by finding the hex code that is most similar (smallest hamming distance) to the code of the given plot.

# Example Usage

Index plots from input PDFs to output CSV index:
```
./plotgrep -o index.csv input_dir/*.pdf
```

Index text from input PDFs to output TXT:
```
 ./plotgrep -o index.txt input_dir/*.pdf
```

Search screen grab of plot in CSV index created above and others:
```
 ./plotgrep index.csv some_other_index_files/*.csv
```

Search text as regex in TXT index created above and others (not implemented yet):
```
 ./plotgrep index.txt some_other_index_files/*.txt
```

## Working principle
Each page of a given PDF is first converted into a black and white bitmap:
[bitmap_page](fig/higgs.pdf_page_007.ppm)
A pixel is assigned to be white if above and black if below a threshold of its grayscale brightness value.
The threshold is determined by the average brightness of all pixels in the page.

# Setup and Install

Prerequisite C libraries:
```
apt-get install libmupdf-dev ljbig2dec
ln -s /usr/lib/x86_64-linux-gnu/libopenjp2.so.2.3.1 /usr/lib/x86_64-linux-gnu/libopenjp2.so
ln -s /usr/lib/x86_64-linux-gnu/libjbig2dec.so.0 /usr/lib/x86_64-linux-gnu/libjbig2dec.so

```

Install:
```
make
```

For development, python prototyping tests (in `./prototyping/`):

```
pip install PyMuPDF
```

# Getting inputs

## Papers from arxiv
kaggle dataset with all arxiv papers: https://www.kaggle.com/datasets/Cornell-University/arxiv

Install gsutil:

```
sudo apt-get install apt-transport-https ca-certificates gnupg
echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] https://packages.cloud.google.com/apt cloud-sdk main" | sudo tee -a /etc/apt/sources.list.d/google-cloud-sdk.list
curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key --keyring /usr/share/keyrings/cloud.google.gpg add -
sudo apt-get update && sudo apt-get install google-cloud-cli
```

To download a single paper:

```
mkdir axv
gsutil -m cp -r gs://arxiv-dataset/arxiv/arxiv/pdf/2402/ ./axv
```

## Slides from indico
Use [indicoscraper](https://github.com/tofitsch/indicoscraper)

# Notes and TODO for development
* Remove `-ggdb3` from Makefile once debugging done
* /mu
* Add multi-threading for mupdf: https://mupdf.com/docs/examples/multi-threaded.c

