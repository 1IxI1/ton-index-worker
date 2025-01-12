#include "InsertManager.h"
#include "tokens.h"
#include "common/checksum.h"


/* typescript
let code = Cell.fromBoc(
    Buffer.from(
        "b5ee9c720102140100021f000114ff00f4a413f4bcf2c80b0102016202030202cd04050201200e0f04e7d10638048adf000e8698180b8d848adf07d201800e98fe99ff6a2687d20699fea6a6a184108349e9ca829405d47141baf8280e8410854658056b84008646582a802e78b127d010a65b509e58fe59f80e78b64c0207d80701b28b9e382f970c892e000f18112e001718112e001f181181981e0024060708090201200a0b00603502d33f5313bbf2e1925313ba01fa00d43028103459f0068e1201a44343c85005cf1613cb3fccccccc9ed54925f05e200a6357003d4308e378040f4966fa5208e2906a4208100fabe93f2c18fde81019321a05325bbf2f402fa00d43022544b30f00623ba9302a402de04926c21e2b3e6303250444313c85005cf1613cb3fccccccc9ed54002c323401fa40304144c85005cf1613cb3fccccccc9ed54003c8e15d4d43010344130c85005cf1613cb3fccccccc9ed54e05f04840ff2f00201200c0d003d45af0047021f005778018c8cb0558cf165004fa0213cb6b12ccccc971fb008002d007232cffe0a33c5b25c083232c044fd003d0032c03260001b3e401d3232c084b281f2fff2742002012010110025bc82df6a2687d20699fea6a6a182de86a182c40043b8b5d31ed44d0fa40d33fd4d4d43010245f04d0d431d430d071c8cb0701cf16ccc980201201213002fb5dafda89a1f481a67fa9a9a860d883a1a61fa61ff480610002db4f47da89a1f481a67fa9a9a86028be09e008e003e00b0",
        // "b5ee9c7201020d0100029c000114ff00f4a413f4bcf2c80b0102016202030202cc040502037a600b0c02f1d906380492f81f000e8698180b8d8492f81f07d207d2018fd0018b8eb90fd0018fd001801698fe99ff6a2687d007d206a6a18400aa9385d47199a9a9b1b289a6382f97024817d207d006a18106840306b90fd001812881a282178050a502819e428027d012c678b666664f6aa7041083deecbef29385d7181406070093b5f0508806e0a84026a8280790a009f404b19e2c039e2d99924591960225e801e80196019241f200e0e9919605940f97ff93a0ef003191960ab19e2ca009f4042796d625999992e3f60101c036373701fa00fa40f82854120670542013541403c85004fa0258cf1601cf16ccc922c8cb0112f400f400cb00c9f9007074c8cb02ca07cbffc9d05006c705f2e04aa1034545c85004fa0258cf16ccccc9ed5401fa403020d70b01c300915be30d0801a682102c76b9735270bae30235373723c0038e1a335035c705f2e04903fa403059c85004fa0258cf16ccccc9ed54e03502c0048e185124c705f2e049d4304300c85004fa0258cf16ccccc9ed54e05f05840ff2f009003e8210d53276db708010c8cb055003cf1622fa0212cb6acb1fcb3fc98042fb0001fe365f03820898968015a015bcf2e04b02fa40d3003095c821cf16c9916de28210d1735400708018c8cb055005cf1624fa0214cb6a13cb1f14cb3f23fa443070ba8e33f828440370542013541403c85004fa0258cf1601cf16ccc922c8cb0112f400f400cb00c9f9007074c8cb02ca07cbffc9d0cf16966c227001cb01e2f4000a000ac98040fb00007dadbcf6a2687d007d206a6a183618fc1400b82a1009aa0a01e428027d012c678b00e78b666491646580897a007a00658064fc80383a6465816503e5ffe4e840001faf16f6a2687d007d206a6a183faa9040",
        "hex",
    ),
)[0];

let slice = code.beginParse();

console.log("contract methods: ", methods);
*/

td::Result<std::vector<unsigned long long>> parse_contract_methods(td::Ref<vm::Cell> code_cell) {
  // check if first op is SETCP0
  // check if second op is DICTPUSHCONST
  // and load method ids from this dict of methods
  try {
    vm::CellSlice cs = vm::load_cell_slice(code_cell);

    auto first_op = cs.fetch_ulong(8);
    auto expectedCodePage = cs.fetch_ulong(8);
    if (first_op != 0xFF || expectedCodePage != 0) {
      return td::Status::Error("Failed to parse 1. SETCP or codepage is not 0");
    }

    auto second_op = cs.fetch_ulong(13);
    auto is_push_op_flag = cs.fetch_ulong(1);
    if (second_op != 0b1111010010100 || is_push_op_flag != 1) {
      return td::Status::Error("Failed to parse 2. DICTPUSHCONST");
    }

    auto methods_dict_key_len = static_cast<int>(cs.fetch_ulong(10));
    auto methods_dict_cell = cs.fetch_ref();
    vm::Dictionary methods_dict{methods_dict_cell, methods_dict_key_len};
    std::vector<unsigned long long> method_ids;
    auto iterator = methods_dict.begin();
    while (!iterator.eof()) {
      // load 32 bits from the start of the key and cut `methods_dict_key_len` bits as only needed
      auto key = td::BitArray<32>(iterator.cur_pos()).to_ulong() >> (32 - methods_dict_key_len);
      method_ids.push_back(key);
      ++iterator;
    }
    return method_ids;
  } catch (vm::VmError& err) {
    return td::Status::Error(PSLICE() << "Failed to parse contract's method ids " << err.get_msg());
  }
}
