file="pubnotes.csv"

while IFS= read -r id; do
  echo $line
  wget https://atlas.web.cern.ch/Atlas/GROUPS/PHYSICS/PUBNOTES/$id/
  url=`grep "http.*pdf" index.html | sed 's/.*"\(.*\)".*/\1/'`
  wget $url -O $id.pdf
  rm index.html
done < $file
