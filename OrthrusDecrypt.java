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

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import java.math.BigInteger;

import java.util.Arrays;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class OrthrusDecrypt {
	public static byte[] MAGIC = "OrthrusVolumeV01".getBytes();

	private static final byte RB = (byte)0x87;

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
		InputStream stream1 = new FileInputStream(card1);
		InputStream stream2 = new FileInputStream(card2);
		byte[] keyblock1 = new byte[SECTORSIZE];
		byte[] keyblock2 = new byte[SECTORSIZE];
		stream1.read(keyblock1);
		stream2.read(keyblock2);

		/*
		 * Each card's key block consists of:
		 *   0x00-0x0f: The MAGIC constant
		 *   0x10-0x2f: The volume ID (common to both cards)
		 *   0x30-0x4f: The key data (different for each card)
		 *   0x50-0x59: The nonce data (different for each card, used for the *opposite* card)
		 *   0x5a:      The flag (0 for A, 1 for B)
		 */
		if (!Arrays.equals(Arrays.copyOfRange(keyblock1, 0, MAGIC.length), MAGIC)) {
			System.err.println("Card 1 is not an Orthrus volume.");
			return;
		}
		if (!Arrays.equals(Arrays.copyOfRange(keyblock2, 0, MAGIC.length), MAGIC)) {
			System.err.println("Card 2 is not an Orthrus volume.");
			return;
		}
		byte[] volumeID = Arrays.copyOfRange(keyblock1, 16, 48);
		if (!Arrays.equals(Arrays.copyOfRange(keyblock2, 16, 48), volumeID)) {
			System.err.println("Cards are not paired.");
			return;
		}
		if (!((keyblock1[96] == 0) ^ (keyblock2[96] == 0))) {
			System.err.println("There needs to be one A and one B card.");
			return;
		}
		if (keyblock1[96] != 0) {
			// swap A and B
			{
				InputStream swap = stream1;
				stream1 = stream2;
				stream2 = swap;
			}
			{
				byte[] swap = keyblock1;
				keyblock1 = keyblock2;
				keyblock2 = swap;
			}
		}
		byte[] keybytes1 = Arrays.copyOfRange(keyblock1, 48, 80);
		byte[] keybytes2 = Arrays.copyOfRange(keyblock2, 48, 80);
		System.err.println("Key block 1: " + hexString(keybytes1));
		System.err.println("Key block 2: " + hexString(keybytes2));
		System.err.println("Volume ID  : " + hexString(volumeID));

		/*
		 * To make the volume key (the AES key for the entire disk),
		 * we run AES CMAC with an all-zero key over the two blocks
		 * of key data (A first, then B). We use the result as the
		 * AES key for an AES CMAC over the volume ID. The result of
		 * that is the volume key.
		 *
		 * It's a happy coincidence that our key size is the same as
		 * the block size. If it weren't, we'd have to get more
		 * creative.
		 */
		Mac mac = Mac.getInstance("AESCMAC");
		mac.init(new SecretKeySpec(new byte[BLOCKSIZE], "AES")); // all zero key
		mac.update(keybytes1);
		mac.update(keybytes2);
		byte[] intermediate = mac.doFinal();
		mac = Mac.getInstance("AESCMAC");
		mac.init(new SecretKeySpec(intermediate, "AES"));
		mac.update(volumeID);
		byte[] volumeKeyBytes = mac.doFinal();
		System.err.println("Volume Key : " + hexString(volumeKeyBytes));
		SecretKey volumeKey = new SecretKeySpec(volumeKeyBytes, "AES");

		/*
		 * The two 10 byte nonce values are combined with the
		 * logical block number to form a 14 byte nonce for
		 * counter mode, but the 10 bytes used are the 10 bytes
		 * stored on the opposite physical card for each logical block.
		 */
		byte[] nonce1 = Arrays.copyOfRange(keyblock1, 80, 96);
		byte[] nonce2 = Arrays.copyOfRange(keyblock2, 80, 96);
		System.err.println("Nonce A : " + hexString(nonce1));
		System.err.println("Nonce B : " + hexString(nonce2));
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
				stream = stream1;
			else
				stream = stream2;
			if (stream.read(ciphertext) <= 0) break; // If we reach the end of one card, we're done.

			// create the individual nonce for this block.
			byte[] nonce = new byte[BLOCKSIZE];
			System.arraycopy(cardA?nonce2:nonce1, 0, nonce, 0, nonce1.length); // pick the nonce from the other card
			nonce[12] = (byte)(block >> 24);
			nonce[13] = (byte)(block >> 16);
			nonce[14] = (byte)(block >> 8);
			nonce[15] = (byte)(block >> 0);
			byte[] tweak = tweakCipher.doFinal(nonce);

			byte[] plaintext = new byte[SECTORSIZE];
			for(int pos = 0; pos < SECTORSIZE; pos += 16) {
				byte[] subBlock = Arrays.copyOfRange(ciphertext, pos, pos + 16);
				for(int i = 0; i < subBlock.length; i++) subBlock[i] ^= tweak[i];
				subBlock = dataCipher.doFinal(subBlock);
				for(int i = 0; i < subBlock.length; i++) subBlock[i] ^= tweak[i];
				System.arraycopy(subBlock, 0, plaintext, pos, subBlock.length);

				// Now make the next tweak. Left shift...
				boolean carry = false;
				for(int i = tweak.length - 1; i >= 0; i--) {
					boolean saved_carry = (tweak[i] & 0x80) != 0;
					tweak[i] <<= 1;
					tweak[i] |= carry?1:0;
					carry = saved_carry;
				}
				// And if there was a carry, xor in the special RB value at the bottom.
				if (carry) tweak[tweak.length - 1] ^= RB;
			}

			System.out.write(plaintext);
		}
	}
}
