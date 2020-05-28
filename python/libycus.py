import re
from struct import unpack

class IbycusIdtFile:
    def __init__(self, id, vol):
        self.Filename_  = id + ".IDT"
        self.Idt_ = open(vol + '/' + self.Filename_, "rb")
        self.authors_ = list()

        c = unpack('B', self.Idt_.read(1))[0]
        while c == 1:
            self.authors_.append(ibyIdtAuth(self.Idt_))
            c = unpack('B', self.Idt_.read(1))[0]

	if c != 0:
	    raise NameError, "Unexpected byte (" + str(c) + " encountered. Expected 1 or 0."
            

    def __del__(self): self.Idt_.close()

    def Count(self): return len(self.authors_)
    def AuthName(self, auth): return self.authors_[auth].Name()
    def AuthId(self, auth):    return self.authors_[auth].Id()
    def AuthBlock(self, auth): return self.authors_[auth].Block()
    def WorkCount(self, auth): return self.authors_[auth].Count()
    def WorkName(self, auth, work): return self.authors_[auth].WorkName(work)
    def WorkId(self, auth, work): return self.authors_[auth].WorkId(work)
    def WorkBlock(self, auth): return self.authors_[auth].WorkBlock(work)
    def SectionCount(self, auth, work): return self.authors_[auth].SectionCount(work)
    def SectionBlock(self, auth, work, sect): return self.authors_[auth].SectionBlock(work, sect)
    def StartId(self, auth, work, sect): return self.authors_[auth].StartId(work,sect)
    def EndId(self, auth, work, sect): return self.authors_[auth].EndId(work, sect)
    def BlockCount(self, auth, work, sect): return self.authors_[auth].BlockCount(work,sect)
    def LastId(self, auth, work, sect, block): return self.authors_[auth].LastId(work, sect, block)
    def IdExceptionCount(self, auth, work, sect): return self.authors_[auth].IdExceptionCount(work, sect)
    def IdException(self, auth, work, sect, index): return self.authors_[auth].IdException(work, sect, index)


class ibyIdtAuth:
    def __init__(self, idt):
        self.block     = 0
        self.length    = 0
        self.start_pos = 0
        self.name_     = ""

        self.length, self.block = unpack('>HH', idt.read(4))

        self.start_pos = idt.tell()
        src = list(unpack(str(self.length) + "B", idt.read(self.length)))
#	src = list(idt.read(self.length)[:])

        # Get the ID of the Author
        self.id_ = IbycusID(src)
        
        # Next should be a description
        byte = src.pop(0)

        if (byte != 0x10):
            # We were expecting a description of id fields a or b
            # Throw some error
            raise NameError, "Throw an error: byte was " + str(byte) + ", not 16" 
 
        byte = src.pop(0)

        if (byte != 0):
            # since we are reading the author, we were expecting level 0
            # Throw some error
	    raise NameError, "Unexpected byte (" + str(byte) + " encountered. Expected 0."

        # Read the name of the Author.
        # The next byte is the length of the name which follows
        for i in range(0, src.pop(0)):
            self.name_ += chr(src.pop(0))

        # The list of works should follow
        self.works_ = list()
        read_authors = 1
        while read_authors:
            byte = src.pop(0)
            if byte == 0x02:
                self.works_.append(ibyIdtWork(src, self.id_))
            elif byte == 0x00:
                # End of File
                read_authors = 0
            else:
                raise NameError, "Throw an error: byte was " + str(byte) + ", not 0 or 2"

    def Name(self):  return self.name_
    def Id(self):    return self.id_
    def Count(self): return len(self.works_)
    def Block(self): return self.block_
    def WorkName(self, work): return self.works_[work].Name()
    def WorkId(self, work): return self.works_[work].Id()
    def WorkBlock(self, work): return self.works_[work].Block()
    def SectionCount(self, work): return self.works_[work].Count()
    def SectionBlock(self, work, sect): return self.works_[work].SectionBlock(sect)
    def StartId(self, work, sect): return self.works_[work].StartId(sect)
    def EndId(self, work, sect): return self.works_[work].EndId(sect)
    def BlockCount(self, work, sect): return self.works_[work].BlockCount(sect)
    def LastId(self, work, sect, block): return self.works_[work].LastId(sect, block)
    def IdExceptionCount(self, work, sect): return self.works_[work].IdExceptionCount(sect)
    def IdException(self, work, sect, index): return self.works_[work].IdException(sect, index)
    
 
class ibyIdtWork:
    def __init__(self, src, parent_id):
        self.name_ = ""
        self.levelDesc_ = {}
        self.sections_ = list()
        self.length = src.pop(0) << 8 | src.pop(0)
        self.block  = src.pop(0) << 8 | src.pop(0)

        # Get the ID of the Author
        self.id_ = IbycusID(src, parent_id.Ids_)
        
        # Next should be a description
        byte = src.pop(0)

        if (byte != 0x10):
            # We were expecting a description of id fields a or b
            # Throw some error
            raise NameError, "Throw an error: byte was " + str(byte) + ", not 16" 
 
        byte = src.pop(0)

        if (byte != 1):
            # since we are reading the work, we were expecting level 1
            # Throw some error
            raise NameError, "Throw an error: byte was " + str(byte) + ", not 1" 

        # Read the name of the work. The next byte should be the
        # length of the string which follows
        for i in range(0, src.pop(0)):
            self.name_ += chr(src.pop(0))

        # Next should be a description of levels n, v..z
        if src[0] != 0x11:
            # We were expecting a description of id fields n, v..z
            # Throw some error
            raise NameError, "Throw an error: byte was " + str(src[0]) + ", not 17" 

        while src[0] == 0x11:
            src.pop(0)
            level_byte = src.pop(0)
            string_len = src.pop(0)
            tmp = ""
            for i in range(0, string_len):
                tmp += chr(src.pop(0))
            self.levelDesc_[level_byte] = tmp

        # Next should be the sections
        if (src[0] != 0x03):
            # Throw some error
            raise NameError, "Throw an error: byte was " + str(byte) + ", not 3"
        last_ids = {}
        while src[0] == 0x03:
            src.pop(0)
            self.sections_.append(ibyIdtSection(src, last_ids))
            last_ids = self.sections_[-1].EndId_.Ids_

    def Name(self):  return self.name_
    def Id(self):    return self.id_
    def Count(self): return len(self.sections_)
    def Block(self): return self.block_
    def SectionBlock(self, sect): return self.sections_[sect].Block()
    def StartId(self, sect): return self.sections_[sect].StartId()
    def EndId(self, sect): return self.sections_[sect].EndId()
    def BlockCount(self, sect): return self.sections_[sect].BlockCount()
    def LastId(self, sect, block): return self.sections_[sect].LastId(block)
    def IdExceptionCount(self, sect): return self.sections_[sect].IdExceptionCount()
    def IdException(self, sect, index): return self.sections_[sect].IdException(index)

class ibyIdtSection:
    def __init__(self, src, last_ids = {}):
        self.block_ = (src.pop(0) << 8) | src.pop(0)
        self.LastIds_ = list()
        self.Exceptions_ = list()

        is_section = 1

        while is_section:
            if src[0] == 0x08:
                # Start ID
                src.pop(0)
                self.StartId_ = IbycusID(src, last_ids)
                last_ids = self.StartId_.Ids_
            elif src[0] == 0x09:
                # End ID
                src.pop(0)
                self.EndId_ = IbycusID(src, last_ids)
            elif src[0] == 0x0a:
                # Last ID for block
                src.pop(0)
                self.LastIds_.append(IbycusID(src, last_ids))
                last_ids = self.LastIds_[-1].Ids_
            elif src[0] == 0x0b:
                # Exception
                src.pop(0)
                self.Exceptions_.append(ibyIdException(src, last_ids))
                last_ids = self.Exceptions_[-1].LastIds()
	    # 0xC, Exception end, is handled in ibyIdException
            elif src[0] == 0x0d:
                # Single Exception
                src.pop(0)
                self.Exceptions_.append(ibyIdException(src, last_ids))
                last_ids = self.Exceptions_[-1].LastIds()
            else:
                is_section = 0

    def StartId(self): return self.StartId_
    def EndId(self): return self.EndId_
    def BlockCount(self): return len(self.LastIds_)
    def Block(self): return self.block_
    def LastId(self, block): return self.LastIds_[block]
    def IdExceptionCount(self): return len(self.Exceptions_)
    def IdException(self, index): return self.Exceptions_[index]

class ibyIdNumber:
    def __init__(self, src, byte, ids, binary = 0, ascii = ""):
        self.binary = binary
        self.ascii = ascii

        if byte:
            rnibble = byte & 0x0F
            if (rnibble == 0):
                # increment this level
                self.binary += 1
            elif (rnibble >= 1 and rnibble <= 7):
                self.binary = rnibble
                self.ascii = ""
            elif rnibble == 0x8:
                # 7-bit binary value
                self.binary = src.pop(0) & 0x7f
                self.ascii = ""
            elif rnibble == 0x9:
                # 7-bit binary value + single ascii character
                self.binary = src.pop(0) & 0x7f
                self.ascii = chr(src.pop(0) & 0x7f)
            elif rnibble == 0xA:
                # 7-bit binary value + ascii string
                self.binary = src.pop(0) & 0x7f
                self.ascii = self.GetAsciiStr(src)
            elif rnibble == 0xB:
	        # 14-bit binary value
	        self.binary = ((src.pop(0) & 0x7f) << 7) | (src.pop(0) & 0x7f)
                ascii = ""
            elif rnibble == 0xC:
                # 14-bit binary value + single ascii character
	        self.binary = ((src.pop(0) & 0x7f) << 7) | (src.pop(0) & 0x7f)
                self.ascii = chr(src.pop(0) & 0x7f)
            elif rnibble == 0xD:
                # 14-bit binary value + ascii string
	        self.binary = ((src.pop(0) & 0x7f) << 7) | (src.pop(0) & 0x7f)
                self.ascii = self.GetAsciiStr(src)
            elif rnibble == 0xF:
                # no binary value + ASCII string
                self.binary = 0
                self.ascii = self.GetAsciiStr(src)
            else:
                raise NameError, "Unhandled rnibble: " + str(rnibble)


    def GetAsciiStr(self,src):
        ascii = ""
        while src[0] >= 0x80:
            byte = src.pop(0)
            if byte == 0xFF:
                break
            ascii += chr(byte & 0x7f)

        return ascii

    def ToString(self):
        if self.binary:
            if self.ascii:
                return str(self.binary) + self.ascii
            else:
                return str(self.binary)
        else:
            return self.ascii

class IbycusID:

    def __init__(self, src, parent_ids = {}):
        # We have to make a copy or else we'll be modifying
	# the ids of the parent
        self.Ids_ = parent_ids.copy()
        self.Descrip_ = {} 

        while src[0] >= 0x80:
            byte = src.pop(0)
            lnibble = (byte & 0xF0) >> 4

            if lnibble == 0xE:
                # Escape code
                level_byte = src.pop(0) & 0x7f
                if level_byte == 0x0:
                    self.Ids_["a"] = ibyIdNumber(src, byte, "")
                elif level_byte == 0x1:
                    self.Ids_["b"] = ibyIdNumber(src, byte, "")
                else:
                    raise NameError, "Unexpected level byte: " + str(level_byte)
            elif lnibble == 0x8:
                self.saveLevel(src, byte, 'z')
            elif lnibble == 0x9:
                self.saveLevel(src, byte, 'y')
            elif lnibble == 0xA:
                self.saveLevel(src, byte, 'x')
            elif lnibble == 0xB:
                self.saveLevel(src, byte, 'w')
            elif lnibble == 0xC:
                self.saveLevel(src, byte, 'v')
            elif lnibble == 0xD:
                self.saveLevel(src, byte, 'n')
            else:
                raise NameError, "UnhandledID " + str(lnibble)
    
    def saveLevel(self, src, byte, level):
	if self.Ids_.has_key(level):
	    binary = self.Ids_[level].binary
	    ascii  = self.Ids_[level].ascii
	else:
	    binary = 0
	    ascii  = ""

	self.Ids_[level] = ibyIdNumber(src, byte, "", binary, ascii)
	# When a higher level (lower letter) is set it implicitly sets
	# all lower levels (higher letters) to 1
        for i in range(ord(level)+1, 123):
            self.Ids_[chr(i)] = ibyIdNumber(0, 0, "", 1, "")

    def ToString(self):
        tmp = list()
        keys = self.Ids_.keys()
        keys.sort()
        for k in keys:
            tmp.append(self.Ids_[k].ToString())
        return str.join(".", tmp)
                
class ibyIdException:
    def __init__(self, src, parent_ids={}):
        self.Block_  = src.pop(0) << 8 | src.pop(0)
        self.StartId_ = IbycusID(src, parent_ids)
        if src[0] == 0xC:
	    src.pop(0)
            self.EndId_ = IbycusID(src, self.StartId_.Ids_)
        else:
            self.EndId_ = ""

    def StartId(self): return self.StartId_
    def Block(self):   return self.Block_
    def EndId_(self):  return self.EndId_
    def IsSingle(self): 
        if self.EndId_:
            return 0
        return 1
    def LastIds(self):
        if self.IsSingle():
            return self.StartId_.Ids_
        return self.EndId_.Ids_


class IbycusAuthTab:
    def __init__(self,dn,list_filename = 'AUTHTAB.DIR'):
        self.list_filename = dn + list_filename
        list_name_length = 4
        src = open(self.list_filename, "rb")
        self.corpora_ = list()

        tag = src.read(list_name_length)
        while tag != "*END":
            self.corpora_.append(ibyAuthTabCorpus(src, tag))
            tag = src.read(list_name_length)

        
        src.close()

    def CorporaCount(self):
        return len(self.corpora_)

    def AuthCount(self, index):
        return self.corpora_[index].Count()

    def CorpusName(self, index):
        return self.corpora_[index].Name()

    def AuthName(self, corpus, author):
        return self.corpora_[corpus].AuthName(author)

    def AuthId(self, corpus, author):
        return self.corpora_[corpus].AuthId(author)

class ibyAuthTabCorpus:
    def __init__(self, src, tag):
        if tag == "tag":
            return
        self.tag_    = tag
        self.name_   = ""
	self.start_pos, self.length = unpack('>HH', src.read(4))

        count = 0
	self.authlist_ = list()
        for author in src.read(self.length-8).split("\xff"):
            if author:
                if count == 0:
                    name = author.split("\x83")
                    self.name_    = name[0]
		    self.alpha_   = name[1]
                else:
                    self.authlist_.append(ibyAuthTabAuthor(author))
            count = count+1

    def Count(self):
        return len(self.authlist_)

    def Name(self):
        return self.name_

    def AuthName(self, index):
        return self.authlist_[index].Name()

    def AuthId(self, index):
        return self.authlist_[index].Id()

class ibyAuthTabAuthor:
    def __init__(self, authstr):

        self.id_      = ''
	self.name_    = ''
        self.aliases_ = list()
	self.alpha_   = ''
	self.comment_ = ''

        # High-bit delimiters
        p = re.compile('[\x80-\xFF]')

	# We know that the string begins with the name
        field = "name"

        # Look for delimiters
        while authstr:
            m = p.search(authstr)
            if m:
	        field_contents = authstr[0:m.start()]
                authstr = authstr[m.end():]
            else:
	       field_contents = authstr
               authstr = ""

            if field == "name":
                self.id_, self.name_ = field_contents.split(' ', 1)
            elif field == "comment":
                self.comment_ = field_contents
            elif field == "alias":
                self.aliases_.append(field_contents)
            elif field == "alphabet":
	        alpha_ = field_contents

            if m:
                if ord(m.group()) == 0x80:
                    field = "alias"
                elif ord(m.group()) == 0x81:
                    field = "comment"
                elif ord(m.group()) == 0x82:
                    field = "unknown"
                elif ord(m.group()) == 0x83:
                    field = "alphabet"
                else:
                    raise NameError, "unknown! " + m.group()
                    field = "unknown"

    def Name(self):
        return self.name_

    def Id(self):
        return self.id_

    def Alpha(self):
        return self.alpha_

    def AliasCount(self):
        return len(self.aliases_)

    def Alias(self,alias):
        return self.aliases_[alias]


         


        
            
            


 






class IbycusTxtFile:
    def __init__(self, id, vol):
        self.Filename_ = id + ".TXT"
        self.Txt_ = open(vol + '/' + self.Filename_, "rb")

    def __del__(self):
        self.Txt_.close()


