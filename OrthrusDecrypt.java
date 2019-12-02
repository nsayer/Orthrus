/*

 Copyright 2017 Nicholas W. Sayer

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * This program is a test for Orthrus. It will take two files on the command line.
 * The two files are intended to be dumps of a pair of Orthrus cards. The two
 * cards will be properly intertwined and decrypted and the result written
 * to standard-out.
 *
 * This is intended to be a test of the hardware/firmware. If you take a partial
 * dump of two cards that are formatted as a proper volume, the decrypted
 * output should be sensible.
 */

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.security.GeneralSecurityException;
import java.security.Security;

import java.nio.ByteBuffer;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.IOException;

import java.math.BigInteger;

import java.util.Arrays;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class OrthrusDecrypt {
	private static final String CMAC_ALG_NAME = "AESCMAC";

	private static class Keyblock {
		public static final byte[] MAGIC;
		static {
			try {
				MAGIC = "OrthrusVolumeV02".getBytes("US-ASCII");
			}
			catch(IOException ex) {
				throw new RuntimeException("This should never be possible.");
			}
		}
		/*
		 * The format of a volume key block is:
		 * 0x00-0x10: magic
		 * 0x10-0x4f: Volume ID
		 * 0x50-0x6f: Key data
		 * 0x70-0x7f: Nonce
		 * 0x80: Card mark - 0 for A, 1 for B
		 */
		public Keyblock(byte[] diskblock) {
			if (diskblock.length != SECTORSIZE)
				throw new IllegalArgumentException("Expect a whole disk block for the Keyblock.");
			ByteBuffer buf = ByteBuffer.wrap(diskblock);
			byte[] magic = new byte[MAGIC.length];
			buf.get(magic);
			if (!Arrays.equals(MAGIC, magic)) throw new IllegalArgumentException("Bad magic.");
			volid = new byte[0x40];
			buf.get(volid);
			keydata = new byte[0x20];
			buf.get(keydata);
			nonce = new byte[0x10];
			buf.get(nonce);
			byte flag = buf.get();
			cardA = flag == 0;
		}
		private byte[] volid, keydata, nonce;
		private boolean cardA;
		public byte[] getVolumeID() { return volid; }
		public byte[] getKeyData() { return keydata; }
		public byte[] getNonce() { return nonce; }
		public boolean isCardA() { return cardA; }
	}

	// This multiplies the given buffer by 2 within GF(128)
	private static final byte RB = (byte)0x87;
	private static void galois_mult(byte[] buf) {
		boolean carry = false;
		for(int i = buf.length - 1; i >= 0; i--) {
			boolean saved_carry = (buf[i] & 0x80) != 0;
			buf[i] <<= 1;
			buf[i] |= carry?1:0;
			carry = saved_carry;
		}
		// And if there was a carry, xor in the special RB value at the bottom.
		if (carry) buf[buf.length - 1] ^= RB;
	}

	private static final int SECTORSIZE = 512; // The size of a disk block
	private static final int BLOCKSIZE = 16; // The AES block size.

	// This just makes a printable version of a byte[]
	private static String hexString(byte[] bytes) {
		BigInteger bi = new BigInteger(1, bytes);
		String out = bi.toString(16);
		int lengthDiff = bytes.length - out.length() / 2;
		for(int i = 0; i < lengthDiff; i++)
			out = "0" + out;
		return out;
	}

	// Concatenate two byte arrays together.
	private static byte[] concat(byte[] a, byte[] b) {
		byte[] out = new byte[a.length + b.length];
		System.arraycopy(a, 0, out, 0, a.length);
		System.arraycopy(b, 0, out, a.length, b.length);
		return out;
	}

	public static void main(String[] args) throws Exception {
		Security.addProvider(new BouncyCastleProvider());

		if (args.length != 2) {
			System.err.println("Need two filenames");
			return;
		}
		File card1 = new File(args[0]);
		File card2 = new File(args[1]);
		if (!card1.canRead() || !card2.canRead()) {
			System.err.println("Can't read the file(s).");
			return;
		}
		try(InputStream stream1 = new FileInputStream(card1)) {
			try(InputStream stream2 = new FileInputStream(card2)) {
				byte[] blockbuf = new byte[SECTORSIZE];
				Keyblock keyblock1, keyblock2;
				try {
					if (stream1.read(blockbuf) != blockbuf.length)
						throw new IllegalArgumentException("Card A not at least one block long.");
					keyblock1 = new Keyblock(blockbuf);
				}
				catch(RuntimeException ex) {
					throw new Exception("Caught exception reading key block from card A.", ex);
				}
				try {
					if (stream2.read(blockbuf) != blockbuf.length)
						throw new IllegalArgumentException("Card B not at least one block long.");
					keyblock2 = new Keyblock(blockbuf);
				}
				catch(RuntimeException ex) {
					throw new Exception("Caught exception reading key block from card B.", ex);
				}
				if (!Arrays.equals(keyblock1.getVolumeID(), keyblock2.getVolumeID()))
					throw new IllegalArgumentException("Cards have different volume IDs.");

				InputStream streamA, streamB;
				Keyblock keyblockA, keyblockB;
				if (keyblock1.isCardA()) {
					keyblockA = keyblock1;
					keyblockB = keyblock2;
					streamA = stream1;
					streamB = stream2;
				} else {
					// swap A and B
					keyblockA = keyblock2;
					keyblockB = keyblock1;
					streamA = stream2;
					streamB = stream1;
				}

				System.err.println("Key block A: " + hexString(keyblockA.getKeyData()));
				System.err.println("Key block B: " + hexString(keyblockB.getKeyData()));
				System.err.println("Volume ID  : " + hexString(keyblockA.getVolumeID()));

				/*
				 * It's unfortunate that the block size and key size aren't the same.
				 * This requires us to use two CMAC operations to make a new key.
				 * We want to insure that the very first half-operation requires us to
				 * have both key blocks, so we will shuffle them togther (A card first).
				 * We perform a zero-key CMAC over the two halves of the shuffle-buffer
				 * concatenating the results to form the intermediate key.
				 */
				byte[] keydata = new byte[64];
				for(int i = 0; i < keyblockA.getKeyData().length; i++) {
					keydata[2 * i] = keyblockA.getKeyData()[i];
					keydata[2 * i + 1] = keyblockB.getKeyData()[i];
				}
				// Build the intermediate key by CMACing the two halves of the shuffled keydata
				// and concatenating the result
				Mac mac = Mac.getInstance(CMAC_ALG_NAME);
				mac.init(new SecretKeySpec(new byte[32], "AES")); // all zero key
				mac.update(Arrays.copyOfRange(keydata, 0, keydata.length / 2));
				byte[] intermediate1 = mac.doFinal();
				mac.init(new SecretKeySpec(new byte[32], "AES")); // all zero key
				mac.update(Arrays.copyOfRange(keydata, keydata.length / 2, keydata.length));
				byte[] intermediate2 = mac.doFinal();
				byte[] intermediate = concat(intermediate1, intermediate2);

				// Now, using the intermediate key, do CMACs over the two halves of the volume ID,
				// concatenating the result to make the volume key.
				mac = Mac.getInstance(CMAC_ALG_NAME);
				mac.init(new SecretKeySpec(intermediate, "AES"));
				mac.update(Arrays.copyOfRange(keyblockA.getVolumeID(), 0, keyblockA.getVolumeID().length / 2));
				intermediate1 = mac.doFinal();
				mac.init(new SecretKeySpec(intermediate, "AES"));
				mac.update(Arrays.copyOfRange(keyblockA.getVolumeID(), keyblockA.getVolumeID().length / 2, keyblockA.getVolumeID().length));
				intermediate2 = mac.doFinal();
				byte[] volumeKeyBytes = concat(intermediate1, intermediate2);
				SecretKey volumeKey = new SecretKeySpec(volumeKeyBytes, "AES");

				/*
				 * The two 12 byte nonce values are combined with the
				 * logical block number to form a 16 byte nonce for
				 * XEX mode, but the bytes used are the nonce bytes
				 * stored on the opposite physical card for each logical block.
				 */
				System.err.println("Nonce A : " + hexString(keyblockA.getNonce()));
				System.err.println("Nonce B : " + hexString(keyblockB.getNonce()));
				Cipher tweakCipher = Cipher.getInstance("AES/ECB/NoPadding");
				tweakCipher.init(Cipher.ENCRYPT_MODE, volumeKey);
				Cipher dataCipher = Cipher.getInstance("AES/ECB/NoPadding");
				dataCipher.init(Cipher.DECRYPT_MODE, volumeKey);
				for(int block = 0; true; block++) {
					// Read the next block from the correct card.
					byte[] ciphertext = new byte[SECTORSIZE];
					boolean cardA = ((block & 1) == 0);
					InputStream stream;
					if (cardA)
						stream = streamA;
					else
						stream = streamB;
					if (stream.read(ciphertext) <= 0) break; // If we reach the end of one card, we're done.
		
					// create the individual nonce for this block.
					byte[] nonce = new byte[BLOCKSIZE];
					System.arraycopy((cardA?keyblockB:keyblockA).getNonce(), 0, nonce, 0, keyblockA.getNonce().length); // pick the nonce from the other card
					// Overwrite the last 4 bytes with the logical block number.
					nonce[nonce.length - 4] = (byte)(block >> 24);
					nonce[nonce.length - 3] = (byte)(block >> 16);
					nonce[nonce.length - 2] = (byte)(block >> 8);
					nonce[nonce.length - 1] = (byte)(block >> 0);
					byte[] tweak = tweakCipher.doFinal(nonce);

					byte[] plaintext = new byte[SECTORSIZE];
					for(int pos = 0; pos < SECTORSIZE; pos += 16) {
						byte[] subBlock = Arrays.copyOfRange(ciphertext, pos, pos + 16);
						for(int i = 0; i < subBlock.length; i++) subBlock[i] ^= tweak[i];
						subBlock = dataCipher.doFinal(subBlock);
						for(int i = 0; i < subBlock.length; i++) subBlock[i] ^= tweak[i];
						System.arraycopy(subBlock, 0, plaintext, pos, subBlock.length);

						// Now make the next tweak.
						galois_mult(tweak);
					}
		
					System.out.write(plaintext);
				}
			}
		}
	}
}
