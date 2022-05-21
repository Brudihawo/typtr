# A WIP typing trainer with some extended analysis features

## Goals
### General features
- [x] Basic working TUI
- [x] Persistent cumulative data
- [x] Random word selection
- [x] Adaptive word frequencies
- [x] sampling according to character errors
- [ ] sampling according to bigram errrors
- [ ] Restart current typing test by pressing esc
- [ ] Raw output for analysis
- [ ] Some basic plotting scripts / jupyter notebooks in python
- [ ] Persistent By-Lesson data - Maybe entire saved text or only stats

### Customization
- [ ] Read config from config file
- [ ] Configure scales for error adaption
- [ ] Selection for lesson size
- [ ] Custom Word list

Got the top1000 wordlist from [here](https://gist.githubusercontent.com/deekayen/4148741/raw/98d35708fa344717d8eee15d11987de6c8e26d7d/1-1000.txt) and the top3000 wordlist from [here](https://www.ef.com/wwen/english-resources/english-vocabulary/top-3000-words/).

## Quickstart

| Requirements |
|--------------|
| Ninja        |
| clang        |
| gnu make     |

```bash
$ git clone https://github.com/brudihawo/typtr.git
$ cd typtr
$ make
$ ./build/typtr
```
You can substitute `make` with `make debug` for debug build
