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

	private static String hexString(byte[] bytes) {
		BigInteger bi = new BigInteger(1, bytes);
		String out = bi.toString(16);
		int lengthDiff = bytes.length - out.length() / 2;
		for(int i = 0; i < lengthDiff; i++)
			out = "0" + out;
		return out;
	}

	private static byte[] prepareCipherBytes(byte[] nonce, SecretKey volumeKey) throws GeneralSecurityException {
		Cipher c = Cipher.getInstance("AES/ECB/NoPadding");
		byte[] out = new byte[512];
		byte[] ivblock = new byte[16];
		System.arraycopy(nonce, 0, ivblock, 0, nonce.length);
		for(int i = 0; i < out.length / 16; i++) {
			ivblock[14] = (byte)(i >> 8);
			ivblock[15] = (byte)(i >> 0);
			c.init(Cipher.ENCRYPT_MODE, volumeKey);
			System.arraycopy(c.doFinal(ivblock), 0, out, 16 * i, 16);
		}
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
		byte[] keyblock1 = new byte[512];
		byte[] keyblock2 = new byte[512];
		stream1.read(keyblock1);
		stream2.read(keyblock2);
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
		if (!((keyblock1[90] == 0) ^ (keyblock2[90] == 0))) {
			System.err.println("There needs to be one A and one B card.");
			return;
		}
		if (keyblock1[90] != 0) {
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
		Mac mac = Mac.getInstance("AESCMAC");
		mac.init(new SecretKeySpec(new byte[16], "AES")); // all zero key
		byte[] keydata = new byte[64];
		System.arraycopy(keybytes1, 0, keydata, 0, keybytes1.length);
		System.arraycopy(keybytes2, 0, keydata, 32, keybytes1.length);
		mac.update(keydata);
		byte[] intermediate = mac.doFinal();
		mac = Mac.getInstance("AESCMAC");
		mac.init(new SecretKeySpec(intermediate, "AES"));
		mac.update(volumeID);
		byte[] volumeKeyBytes = mac.doFinal();
		System.err.println("Volume Key : " + hexString(volumeKeyBytes));
		SecretKey volumeKey = new SecretKeySpec(volumeKeyBytes, "AES");
		byte[] nonce1 = Arrays.copyOfRange(keyblock1, 80, 90);
		byte[] nonce2 = Arrays.copyOfRange(keyblock2, 80, 90);
		System.err.println("Nonce A : " + hexString(nonce1));
		System.err.println("Nonce B : " + hexString(nonce2));
		for(int block = 0; true; block++) {
			byte[] ciphertext = new byte[512];
			boolean cardA = ((block & 1) == 0);
			InputStream stream;
			if (cardA)
				stream = stream1;
			else
				stream = stream2;
			if (stream.read(ciphertext) <= 0) break;
			byte[] nonce = new byte[14];
			System.arraycopy(cardA?nonce2:nonce1, 0, nonce, 0, nonce1.length);
			nonce[10] = (byte)(block >> 24);
			nonce[11] = (byte)(block >> 16);
			nonce[12] = (byte)(block >> 8);
			nonce[13] = (byte)(block >> 0);
			byte[] cipherBytes = prepareCipherBytes(nonce, volumeKey);
			for(int i = 0; i < ciphertext.length; i++)
				ciphertext[i] ^= cipherBytes[i];
			System.out.write(ciphertext); // now it's the plaintext
		}
	}
}
