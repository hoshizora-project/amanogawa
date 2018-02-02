# :milky_way: amanogawa: Graph construction meets DAG-based data-oriented processing engine
hogehoge

## :sparkles: Features
* Easy to use
  * You can use *amanogawa* as a Python library, C++ library and CLI tool
  * Flexible DAG representation
* Extremely fast
  * Full native speed
  * Empowered by [Apache Arrow][arrow]
* Modular design
  * You can add templates of data source, format, data processing, join, branch, etc. as plugin

## :bulb: Example
### Task1: Simple
Read a single csv, filter and then export to tsv

```csv
id,name
0,Aries
1,Taurus
2,Gemini
3,Cancer
4,Leo
5,Virgo
6,Libra
7,Scorpius
8,Sagittarius
9,Capricornus
10,Aquarius
11,Pisces
```

### Task2: Graph construction

### Task3: Complex
Read a json and csv, join them, split by column and then write them into csv


## :persevere: WIP
* [ ] Automatic input schema config generator, like guess in embulk
* [ ] Out-of-core processing
* [ ] Effective parallel processing and scheduling
* [ ] Dynamic DAG scheduling
* [ ] Effective use of Apache Arrow
* [ ] Row-based and Column-based, compound data handling
* [ ] Data validation and error handling
* [ ] Tests
* [ ] Many many plugins

[arrow]: https://arrow.apache.org