# lang.swig

** swig maiken module **

Compile phase module

## Prerequisites
  [maiken](https://github.com/mkn/mkn)

## Usage

```yaml
mod:
- name: lang.swig
  compile:              # calls swig-config --includes
    src: $file1 $file2  # no default, one file required
    conf: $str          # default: "-python -py3 -c++ -modern -new_repr"
    inc: $dir0 $dir1    # add directories for header scanning
    outdir: $dir        # location of generated integration files - default src directory
    objdir: $dir        # location of generated source file - default src directory
    outfile: $file      # filename override for generated source file
    lang:               # default "cpp"

```

## Building

  Windows cl:

    mkn clean build -tSa -EHsc -d


  *nix gcc:

    mkn clean build -tSa "-O2 -fPIC" -d -l "-pthread -ldl"


## Testing

  Windows cl:

    mkn clean build -tSa -EHsc -dp test run

  *nix gcc:

    mkn clean build -tSa "-O2 -fPIC" -dp test -l "-pthread -ldl" run


## Environment Variables

    Key             SWIG
    Type            string
    Default         ""
    Description     If set - uses string as command call instead of "swig"
