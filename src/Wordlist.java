
import java.io.*;
import java.util.*;


public class Wordlist {

    public static final int
          FOUND_NONE = 0,
          FOUND_EXACT = 1,
          FOUND_PREFIX = 2
		  ;
	public static final byte
		FLAG_VOWEL = 0x01
		;

	public int size_words, count_charset, count_words;
	public String name;

	public char []charset_code, charset_unicode;
	public int []charset_count;
	public byte []charset_flags;
	public Map<Character, Character> map2ascii, map2utf8;

	private static final int MAGIC = 0x776c6462;
	private byte []words;

	public Wordlist(InputStream r) throws IOException {

		if(read32(r) != MAGIC)
			throw new IOException("not a wordlist");

		size_words =  read32(r);
		count_charset =  read32(r);
		count_words =  read32(r);

		// up to 64 bytes of zero-termianted string
		byte [] tmp = new byte[64];
		r.read(tmp);
		name = new String(tmp);

		System.out.println("Loading " + name + ", " + count_words + " words in " +
			count_charset + " letters and " + size_words + " bytes");

		words = new byte[size_words];

		r.read(words);


		map2utf8 = new TreeMap();
		map2ascii = new TreeMap();

		charset_code = new char[count_charset];
		charset_unicode = new char[count_charset];
		charset_count = new int[count_charset];
		charset_flags = new byte[count_charset];

		for(int i = 0; i < count_charset; i++) {
			charset_code[i] = (char) r.read();
			charset_flags[i] = (byte) r.read();
			charset_count[i] = read32(r);
			charset_unicode[i] = (char) read32(r);

			map2utf8.put(charset_code[i], charset_unicode[i]);
			map2ascii.put(charset_unicode[i], charset_code[i]);

			System.out.printf(" %d: %c -> %c flags=%02x count=%d\n", i, charset_code[i] , charset_unicode[i], charset_flags[i], charset_count[i]);

		}
	}

	/*  convert from ascii to utf8 or vice versa */
	public char []convert(char []src, boolean ascii2utf8)
	{
		char []dst = new char[src.length];
		for(int i = 0; i < dst.length; i++) {
			Character c = (ascii2utf8 ? map2utf8 : map2ascii) .get(src[i]);
			dst[i] = c == null ? src[i] : c;
		}
		return dst;
	}

	/* lookup an standard java string */
	public int lookup(String str)
	{
		char []aschars = convert( str.toCharArray(), false);
		byte []asbytes = new byte[ aschars.length];

		for(int i = 0; i < asbytes.length; i++)
			asbytes[i] = (byte)aschars[i];

		return lookup(asbytes, asbytes.length);
	}

	/* lookup an ascii string as bytes */
	public int lookup(final byte [] t, final int tlen)
    {
        final byte [] words = this.words;
        final int wlen = this.words.length;

        int mid, low = 1, high = wlen -1;
        int tmp;
        boolean partial = false;

        while(low < high) {
            tmp = mid = (low + high) / 2;
            while(tmp > 0 && words[tmp] != 0)
                tmp--;
            tmp++;

            int k = strcmp(words, tmp, t, tlen);
            if( k == 0 && words[tmp+tlen] != 0) {
                partial = true;
                k = +1;
            }

            if(k == 0) return FOUND_EXACT;
            else if(k < 0) low = mid + 1;
            else high = mid;
        }
        return partial ? FOUND_PREFIX : FOUND_NONE;
    }

    private static final int strcmp(byte [] list, int offset, byte []t, int tlen)
    {
        for(int i = 0; i < tlen; i++) {
            byte b1 = t[i];
            byte b2 = list[offset++];
            if(b1 < b2) return +1;
            if(b2 < b1) return -1;
        }
        return 0;
    }
	private static final int read32(InputStream r) throws IOException {
		int n = 0;
		for(int i = 0; i < 4; i++)
			n = (n << 8) | (0xFF & r.read());
		return n;
	}
}

class WordlistTest {
	public static void main(String []args) {

		if(args.length != 1) {
			System.out.println("No wordlist was given.");
			System.exit(20);
		}
		try {
			InputStream i = new FileInputStream(args[0]);
			Wordlist wl = new Wordlist(i);
			i.close();

			// now test it!
			final String [] result_text = { "NOT FOUND", "FOUND", "FOUND (as prefix)" };


			System.out.println("Enter word to lookup (press ^C or write . to quit): ");
			Scanner s = new Scanner(System.in);
			while(s.hasNext()) {
				String str = s.next();
				if(".".equals(str))
					break;

				long t = System.currentTimeMillis();
				int n = wl.lookup(str);
				t = System.currentTimeMillis() - t;

				System.out.printf("Searched %s in %d ms -> %s\n",
					str, t, result_text[n]);
			}
		}catch(IOException e) {
			System.err.println(e);
		}
	}
}