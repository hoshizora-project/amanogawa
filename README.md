# :milky_way: amanogawa: Graph construction meets DAG-based data processing engine
Flexible graph construction and data pre-processing engine

Tutorial is [here](https://github.com/hoshizora-project/tutorial).
You can try *amanogawa* and [hoshizora](https://github.com/hoshizora-project/hoshizora) on Jupyter on Docker

**:warning: Currently alpha version. Inner structure and APIs might be change a lot**


## :sparkles: Features
* Easy to use
  * You can use *amanogawa* as a Python library, C++ library and CLI tool
  * Flexible DAG representation
* Extremely fast
  * Full native speed
  * Empowered by [Apache Arrow](https://arrow.apache.org)
* Modular design
  * You can add templates of data source, format, data processing, join, branch, etc. as plugin


## :soon: Install
Supporting Linux and macOS

### Python library via pip
```sh
pip install amanogawa
```

### From source
Prerequisites
* Make
* CMake 3.0+ 
* Clang++ 3.4+
* Python 3

```sh
make init
```

#### CLI
```sh
make release
```

#### Python library
```sh
python3 setup.py install
```

## :bulb: Example
### Task:one:: Simple (Python)
Read a single json, filter and then export to csv

`sample.json`
```json
[
  {"id": 1, "name": "Aries"},
  {"id": 2, "name": "Taurus"},
  {"id": 3, "name": "Gemini"}
]
```

```python
import amanogawa as am
builder = am.ConfigBuilder()
config = builder.source('file').set('path', 'sample.csv').format('csv') \
    .set('columns',
        [{'name': 'id', 'type': 'int'}, {'name': 'name', 'type': 'string'}]) \
    .set('filter', {'key': 'name', op: 'contains', 'cond': 'i'})
    .sink('file').set('path', 'sample.tsv').format('csv').set('delimiter', '\t') \
    .build()
am.execute(config)
```

`sample.csv`
```csv
id,name
1,Aries
3,Gemini
```

### Task:two:: Graph construction (Python)
Read json lines, construct graph and then export to csv

`comments.jsonl`
```jsonl
{"content": "Apple Strawberry Apple", "command": "foo"}
{"content": "Apple Strawberry", "command": "foo"}
{"content": "Apple Apple", "command": "bar"}
{"content": "Banana Banana", "command": "foo bar"}
{"content": "Pineapple Banana Banana", "command": "foo"}
```

```python
import amanogawa as am
builder = am.ConfigBuilder()
config = builder.source('file').set('path', 'comments.jsonl').format('json') \
    .set('columns', [{'name': 'content', 'type': 'string'}]) \
    .flow('to_graph').set('mode', 'bow').set('column', 'content').set('knn', {'k': 2, 'p': 1.5}) \
    .sink('file').set('path', 'graph').format('csv').set('delimiter', ' ').build()
am.execute(config)
```

```
src dst
0 4
0 3
0 2
1 4
1 3
1 2
2 4
2 3
```

### Task:three:: Complex (CLI)
Read csvs, join them, split by column and then export to csv and tsv

#### In
`kinmosa.csv`
```csv
id,name,blood_id
1,karen,3
2,ayaya,0
3,shino,0
4,yo-ko,2
5,alice,0
```

`blood.csv`
```csv
id,type
0,A
1,B
2,O
3,AB
```

#### Config
`config.toml`
```toml
[source.read_awesome_csv]
type = "file"
path = "kinmosa.csv"
[source.read_awesome_csv.format]
type = "csv"
columns = [
  { name = "id", type = "int" },
  { name = "name", type = "string" },
  { name = "blood_type", type = "int" }
]

[branch.id_name_blood]
type = "column"
from = "read_awesome_csv"
to = [
  { name = "id_name", columns = [ "id", "name" ] },
  { name = "blood", columns = [ "blood_type" ] }
]

[source.about_blood]
type = "file"
path = "blood.csv"
[source.about_blood.format]
type = "csv"
columns = [
  { name = "id", type = "int" },
  { name = "type_string", type = "string" }
]

[confluence.blood_type]
type = "key"
from = [
  { name = "about_blood", key = "id" },
  { name = "blood", key = "blood_type" }
]

[sink.write_id_name_tsv]
type = "file"
path = "result_id_name.tsv"
from = "id_name"
[sink.write_id_name_tsv.format]
type = "csv"
delimiter = "\t"

[sink.write_blood_csv]
type = "file"
path = "result_blood.csv"
from = "blood_type"
[sink.write_blood_csv.format]
type = "csv"
```

```sh
./amanogawa-cli config.toml
```

#### Out
`result_id_name.csv`
```tsv
id	name
1	karen
2	ayaya
3	shino
4	yo-ko
5	alice
```

`result_blood.csv`
```csv
id,type_string
0,A
0,A
0,A
2,O
3,AB
```


## :persevere: WIP
* [ ] Support files with serial number
* [ ] Efficient config builder
* [ ] Automatic input schema config generator, like guess in embulk
* [ ] Out-of-core processing
* [ ] Effective parallel processing and scheduling
* [ ] Dynamic DAG scheduling
* [ ] Effective use of Apache Arrow (Currently using it as just an interface)
* [ ] Row-based and Column-based, compound data handling
* [ ] Data validation and error handling
* [ ] Sharing amanogawa-core between plugins
* [ ] Tools for creating third-party plugins
* [ ] Tests
* [ ] Many many plugins
