TeMpLe---Typed Markup Language
==============================

Comparison between data formats
-------------------------------

|  | XML | JSON | TeMpLe | HDF5 |
|:----------------------:|:-----------------------------------------:|:-------------------------------------------:|:--------------------------------------------------------------:|:-----------------------------:|
| Encoding | Text | Text | Text | Binary |
| Compounds | Yes | Yes | Yes | Yes |
| Sequencial data | Yes | Keys: no, Arrays: yes | Keys: no, Arrays: yes | Keys: no, Arrays: yes |
| Native arrays | No | Yes, hetrogenous | Yes, homogenous | Yes, homogenous |
| Type support | Attributes: String, Element: No support | Number and string | Integers, floats, and string | Integers, floats, and strings |
| Serialization overhead | Compounds: element name + attribute names | Arrays: one byte, Compounds: property names | Arrays: one byte, Compounds: property names + type annotations | ? |