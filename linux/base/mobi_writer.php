<?php
/**
 * @author dizaz7@2011
 */
class TextData {
    public $title;
    public $records = array();
    public $author;
    public function getTitle() {
        return $this->title;
    }
    public function getRecords() {
        return $this->records; //a record set
    }
    public function getAuthor() {
        return $this->author;
    }
}

define('FCIS_FLIS', false);
define('RECORD_SIZE', 0x1000);

class WriteMobi {
    private $_data;     //data
    private $_name;     //file name
    private $_stream;   //out stream
    private $_records;  //record set

    public function __construct($name, TextData $data, $fp) {
        try {
            if ($data == null) throw Exception('NOT IS OBJECT(TextData)');
            $this->_data = $data;
            $this->_name = $name;
            //
            $this->_compression = 0;
            $this->_text_length = 0x1000;
            $this->_text_nrecords = 0x3;
            $this->_records = $data->getRecords();
            $this->_stream = $fp;
        } catch (Exception $e) {
            die(sprintf("Error: %s\n", $e->getMessage()));
        }
        //init
        //$this->_records = array();
    }

    private function write() {
        $num_args = func_num_args();
        $arg_list = func_get_args();
        for ($i = 0; $i < $num_args; ++$i) {
            //写入文件
            //$this->_stream->write($arg_list[$i]);
            fwrite($this->_stream, $arg_list[$i]);
        }
    }

    private function tell() {
        //return $this->_stream->tell();
        return ftell($this->_stream);
    }

    private function build_exth() {
        $exth = '';
        // base addr
        // 4 bytes : The chracters E X T H
        $exth .= 'EXTH';
        // 4 bytes : The length of the EXTH header, including the previous 4 bytes
        //@TODO
        $exth .= pack('N', 0x28); // 1c + c = 28
        // 4 bytes : record Count
        // @TODO
        $exth .= pack('N', 0x1); //1 record

        // for () { 可有多项，最多(record count)
        // 4 bytes : record type
        // @TODO
        $exth .= pack('N', 0x67); // 0x67 - distribution 
        // 4 bytes : record length (length = previous(8) + len(L))
        // @TODO
        $exth .= pack('N', 0x1c); // 14 + 8 = 1c 
        // len(L) - 8 bytes : record data L
        // @TODO
        // }
        $exth .= 'This is a test'; //14 bytes
        return $exth;
    }

    private function generate_end_records() {
        if (FCIS_FLIS) {
            $this->_flis_number = count($this->_records);
            $this->_records[] = "FLIS\0\0\0\x08\0\x41\0\0\0\0\0\0\xff\xff\xff\xff\0\x01\0\x03\0\0\0\x03\0\0\0\x01\xff\xff\xff\xff";
            $this->_fcis_number = count($this->_records);
            $this->_records[] = 
                "FCIS\x00\x00\x00\x14\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x00" 
                . pack('N', $this->_text_length) 
                . "\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x08\x00\x01\x00\x01\x00\x00\x00\x00";
            $this->_records[] = "\xE9\x8E\x0D\x0A";
        } else {
            $this->_flis_number = count($this->_records);
            $this->_records[] = "\xE9\x8E\x0D\x0A";
        }
    }

    private function generate_record0() {
        $exth = $this->build_exth();
        $last_content_record = count($this->_records) - 1;
        $record0 = ''; //bin string
        $record0 .= pack('nnNnnnn', 
            $this->_compression, 
            0, 
            $this->_text_length, 
            $this->_text_nrecords - 1, RECORD_SIZE, 0, 0);
        $uid = rand(0, 0xffffffff);
        $title = "This is a test";
        //0x0 - 0x3 : M O B I
        $record0 .= 'MOBI'; //append string 
        //0x4 - 0x7 : Length of header
        //0x8 - 0x11 : MOBI type
        //      type        meaning
        //      0x002       MOBI book(chapter - chapter navigation)
        //      0x101       News - Hierarchical navigation with sections and articles
        //      0x102       News feed - Flat navigation
        //      0x103       News magazine - same as 0x101
        //0xC - 0xF : Text encoding(65001 is utf-8)
        //0x10 - 0x13 : UID
        //0x14 - 0x17 : Generator version

        $btype = 0x002;
        $record0 .= pack('NNNNN', 0xe8, $btype, 65001, $uid, 0x6);

        //0x18 - 0x1f : Unknown
        $record0 .= "\xff\xff\xff\xff\xff\xff\xff\xff";

        //0x20 - 0x23
        $record0 .= pack('N', 0xffffffff);

        //0x24 - 0x3f : Unknown
        //for ($i = 0; $i < 28; ++$i) {
        //    $record0 .= "\xff";
        //}
        $record0 .= str_repeat("\xff", 28);

        //0x40 - 0x43 : Offset of first non-text record 
        $record0 .= pack('N', $this->_text_nrecords + 1);

        //0x44 - 0x4b : title offset, title length
        //@TODO
        $record0 .= pack('NN', 0xe8 + 16 + strlen($exth), strlen($title));

        //0x4c - 0x4f : Language specifier
        //@TODO
        $record0 .= pack('N', 1);

        //0x50 - 0x57 : Unkown
        $record0 .= str_repeat("\0", 8);

        //0x58 - 0x5b : Format version
        //0x5c - 0x5f : First image record number
        $record0 .= pack('NN', 6, $this->_first_image_record ? $this->_first_image_record : 0);

        //0x60 - 0x63 : First HUFF/CDIC record number
        //0x64 - 0x67 : Number of HUFF/CDIC records 
        //0x68 - 0x6b : First DATP record number
        //0x6c - 0x6f : Number of DATP records
        $record0 .= str_repeat("\0", 16);

        //0x70 - 0x73 : EXTH flags
        $record0 .= pack('N', 0x50);

        //0x74 - 0x93 : Unkown
        $record0 .= pack('NNNN', 0x00, 0x00, 0x00, 0x00);

        //0x94 - 0x97 : DRM offset
        //0x98 - 0x9b : DRM count
        //0x9c - 0x9f : DRM size
        //0xa0 - 0xa3 : DRM flags
        $record0 .= pack('NNNN', 0xffffffff, 0xffffffff, 0, 0);

        //0xa4 - 0xaf : Unknown
        $record0 .= str_repeat("\0", 12);

        //0xb0 - 0xb1 : First content record number
        //0xb2 - 0xb3 : last content record number
        //(Includes Image, DATP, HUFF, DRM)
        $record0 .= pack('nn', 1, $last_content_record);

        // 0xb4 - 0xb7 : Unknown
        $record0 .= "\0\0\0\x01";

        //0xb8 - 0xbb : FCIS record number
        if (FCIS_FLIS) {
            // Write these if FCIS/FLIS turned on
            // 0xb8 - 0xbb : FCIS record number
            $record0 .= pack('N', $this->_fcis_number);

            // 0xbc - 0xbf : Unknown (FCIS record count?)
            $record0 .= pack('N', 1);

            //0xc0 - 0xc3 : FLIS record number
            $record0 .= pack('N', $this->_flis_number);

            //0xc4 - 0xc7 : Unknown (FLIS record count?)
            $record0 .= pack('N', 1);
        } else {
            // 0xb8 - 0xbb : FCIS record number
            $record0 .= pack('N', 0xffffffff);

            // 0xbc - 0xbf : Unknown (FCIS record count?)
            $record0 .= pack('N', 0xffffffff);

            //0xc0 - 0xc3 : FLIS record number
            $record0 .= pack('N', 0xffffffff);

            //0xc4 - 0xc7 : Unknown (FLIS record count?)
            $record0 .= pack('N', 1);
        }
        // 0xc8 - 0xcf : Unkown
        $record0 .= str_repeat("\0", 8);

        // 0xd0 - 0xdf : Unkown
        $record0 .= pack('NNNN', 0xffffffff, 0, 0xffffffff, 0xffffffff);

        // 0xe0 - 0xe3 : Extra record data
        // Extra record data flags:
        //      - 0x1: <extra multibyte bytes><size>(?)
        //      - 0x2: <TBS indexing description of this HTML record><size> GR
        //      - 0x4: <uncrossable breaks><size>
        // GR: Use 7 for indexed files, 5 for unindexed
        // Setting bit 2 (0x4) disables <guide><reference type="start"> functionality
        //@TODO
        $record0 .= pack('N', 0x1);

        // 0xe4 - 0xe7 : Primary index record
        $record0 .= pack('N', $this->_primary_index_record ? $this->_primary_index_record : 0xffffffff);
        //@TODO
        $record0 .= $exth;
        //@TODO
        $record0 .= $title;
        $this->_records[0] = $record0 . str_repeat("\0", (1024 * 8));
    }

    private function write_header() {
        $title = $this->_data->getTitle();
        $title = preg_replace('/[^-A-Za-z0-9]/', '_', $title);
        $zero = '';
        for ($i = 0; $i < (32 - strlen($title)); $i++) {
            $zero .= "\0";
        }
        $title = $title . $zero;
        $now = 2011;
        $nrecords = count($this->_records);
        $this->write(
            $title, 
            pack('nnNNNNNN', 0, 0, 0, 0, 0, 0, 0, 0),
            'BOOK',
            'MOBI',
            pack('NNn', 0, 0, 0)
        );
        $offset = $this->tell() + (8 * $nrecords) + 2;
        foreach ((array)$this->_records as $i => $record) {
            $this->write(
                pack('N', $offset), 
                "\0",
                substr(pack('N', 2 * $i), 1)
            );
            $offset += count($record);
        }
        $this->write("\0\0");
    }

    private function write_content() {
        foreach ((array)$this->_records as $record) {
            $this->write($record);
        }
    }
    public function create() {
        $this->build_exth();
        $this->generate_record0(); // record0
        $this->_records[] = "This is a test";
        $this->generate_end_records(); //record-last
        $this->write_header();
        $this->write_content();
    }
}

$data = new TextData();
$data->records = array('this is a test');
$fp = fopen('t.mobi', 'wb+');
$mobi = new WriteMobi('xiangshouding', $data, $fp);
$mobi->create();
fclose($fp);
