TeMpLe---Typed Markup Language
==============================

TeMpLe is a text-based data serialization format, that can represent common data types natively. The  format is heavily inspired by [JSON][1]. While JSON has excellent support for data structures found within a program, it has no support for integers, single-precision floats, homogeneous arrays, and comments.

To add support for integers, [libjansson][2] uses type inference: A number that is a valid integer is treated as an integer. These integers are always represented by the largest signed integer type on the system. It does not support the C style `f` suffix for denoting single precision values.

True support for integers has the advantage that floating point values that happens to be integers, do not need to end with `.0`. It is also good to have support for integers and floats of a different size. This way, values that are to small or to large, will trigger an error during load-time, effectively eliminating any unintended truncation.

While it may be useful with heterogamous arrays, using them as a substitute for homogeneous ones is inefficient. To allow different types for each array element, either a tagged union such as `std::variant` from C++17, or a pointer to a polymorphic object type is needed. The former requires an extra type tag inside each element, while the latter will require use of freestore, slowing down element access. Furthermore, it is unlikely that any system API is compatible with any of these representations. However, a pointer to a homogeneous array can be used directly to functions like [`glBufferData`][3] and similar functions.

Support for comments is good, especially in configuration files. This makes it possible to use a shorter key name, since more descriptive information, such as valid values, can be placed in the comment. Apparently, it was misused in [JSON][4].

Comparison between data formats
-------------------------------

The following table shows a comparison between TeMpLe and other data formats.

|  | XML | JSON | TeMpLe | HDF5 |
|:----------------------:|:-----------------------------------------:|:-------------------------------------------:|:--------------------------------------------------------------:|:-----------------------------:|
| Encoding | Text | Text | Text | Binary |
| Compounds | Yes | Yes | Yes | Yes |
| Comments | Yes | No | Yes | ? |
| Sequencial data | Yes | Keys: no, Arrays: yes | Keys: no, Arrays: yes | Keys: no, Arrays: yes |
| Heterogeneous arrays | No | Yes | No | No |
| Homogeneous arrays | No | No | Yes | Yes |
| Type support | Attributes: String, Element: No support without schema | Number and string | Integers, floats, and string | Integers, floats, and strings |
| Serialization overhead | Compounds: element name + attribute names | Arrays: one byte, Compounds: property names | Arrays: one byte, Compounds: property names + type annotations | ? |
| Compression support | No | No | No | Yes|

Neither TeMpLe or HDF5 can store hetrogenous arrays, however TeMpLe can emulate arrays of compounds through syntactic sugar.

The TeMpLe syntax
-----------------
TeMpLe borrows a great deal of its syntax from JSON. The differences to note are

 * Keys and values do not need to be surrounded by quotation marks. When there are no quotation marks, all whitespace characters are ignored.
 * A key may be followed by a type identifier. If there is no such identifier, it is assumed that the key represents a compound, or an array of compounds.
 * Ant character following on a line an unquoted, or unescaped '#', is a comment

In BNF-like syntax

	<file> ::= { (<compound> | <compound_array>) }
	<member> ::= <pair> | (<pair> "," <member>)
	<pair> ::= <key> ("," <typed_value> | <object> )
	<typed_value> ::= <type_id>  ":" ( <array> | <value> )
	<element> ::= <value> | ( <value> "," <element> )
	<array> ::= "[" {<element>} "]"
	<object> ::= <compound> | <compound_array>
	<compound> ::= { "{" <member> "}" }
	<compound_array> ::= "[" {<compound_element>} "]"
	<compound_element> ::= <compound> | ( <compound> "," <compound_element> )
	<type_id> ::= ("i8"|"i16"|"i32"|"i64"|"f"|"d"|"s")
	


[1]: https://tools.ietf.org/html/rfc7159
[2]: http://www.digip.org/jansson/
[3]: https://www.opengl.org/sdk/docs/man4/html/glBufferData.xhtml
[4]: https://plus.google.com/+DouglasCrockfordEsq/posts/RK8qyGVaGSr
