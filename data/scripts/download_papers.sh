file="papers.csv"

args=()

while IFS= read -r line; do
  args+=("gs://arxiv-dataset/arxiv/arxiv/pdf/$line")
done < $file

gsutil -m cp -n ${args[@]} .
