https://www.kaggle.com/datasets/Cornell-University/arxiv

```
sudo apt-get install apt-transport-https ca-certificates gnupg
echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] https://packages.cloud.google.com/apt cloud-sdk main" | sudo tee -a /etc/apt/sources.list.d/google-cloud-sdk.list
curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key --keyring /usr/share/keyrings/cloud.google.gpg add -
sudo apt-get update && sudo apt-get install google-cloud-cli
mkdir axv
gsutil -m cp -r gs://arxiv-dataset/arxiv/arxiv/pdf/2402/ ./axv
```

Note: remove -ggdb3 from Makefile once debugging done

For python tests:

```
pip uninstall PyMuPDF
pip install PyMuPDF
```

## indico

* Create API key at https://indico.cern.ch/user/api/
* copy it into `indico_api.secret`

