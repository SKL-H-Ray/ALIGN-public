import filecmp
import pathlib

from align.gdsconv.gds2prettyjson import convert_GDS_GDSprettyjson
from align.gdsconv.json2gds import convert_GDSjson_GDS

mydir = pathlib.Path(__file__).resolve().parent

def test_gds_json_roundtrip ():
    convert_GDS_GDSprettyjson (mydir / "file.gds", mydir / "fromgds.json")
    convert_GDSjson_GDS (mydir / "fromgds.json", mydir / "fromjson.gds")
    assert (filecmp.cmp (mydir / "file.gds", mydir / "fromjson.gds"))

def test_json_gds_roundtrip ():
    convert_GDSjson_GDS (mydir / "file.pretty.json", mydir / "fromjson2.gds")
    convert_GDS_GDSprettyjson (mydir / "fromjson2.gds", mydir / "fromgds2.json")
    assert (filecmp.cmp (mydir / "file.pretty.json", mydir / "fromgds2.json"))

