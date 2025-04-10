import java.io.FileOutputStream;
import java.math.BigInteger;
import java.security.*;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Date;
import javax.security.auth.x500.X500Principal;

import org.bouncycastle.cert.jcajce.JcaX509CertificateHolder;
import org.bouncycastle.cert.jcajce.JcaX509v3CertificateBuilder;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.CertIOException;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.openssl.jcajce.JcaPEMWriter;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class CertificateGenerator {
  public static void generateKeyPairAndCABundle(String keyStoreFilePath, String keyStorePassword) throws Exception {
    // Bouncy Castle as the security provider
    Security.addProvider(new BouncyCastleProvider());

    // Generate the key pair
    KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
    keyPairGenerator.initialize(2048, new SecureRandom());
    KeyPair keyPair = keyPairGenerator.generateKeyPair();

    // Define the certificate validity
    long notBefore = System.currentTimeMillis();
    long notAfter = notBefore + 30*24*60*60*1000L; // One month validity

    X500Principal subject= new X500Prinicpal("CN=Sushanta, ST=Odisha, C=India");
    X500Principal signedByPrincipal = subject;
    BigInteger certSerialNumber = BigInteger.valueOf(System.currentTimeMillis());

    // Create the certificate Builder
    ContentSigner contentSigner = new JcaContentSignerBuilder("SHA256WithRSAEncryption").build(keyPair.getPrivate());
    X509v3CertificateBuilder certBuilder = new JcaX509v3CertificateBuilder(
        signedByPrincipal, certSerialNumber, new Date(notBefore), new Date(notAfter), subject, keyPair.getPublic());
    X509CertificateHolder certHolder = certBuilder.build(contentSigner);
    X509Certificate certificate = new JcaX509CertificateHolder(certHolder).getCertificate();

    // Save the CA certificate and private key in a KeyStore
    KeyStore keyStore = KeyStore.getInstance("JKS");
    keyStore.load(null, null); // Initialize an empty keystore

    // Set up the KeyStore entry for the CA certificate
    KeyStore.PrivateKeyEntry entry = new KeyStore.PrivateKeyEntry(keyPair.getPrivate(), new Certificate[] {certificate});
    KeyStore.PasswordProtection password = new KeyStore.PasswordProtection(keyStorePassword.toCharArray());
    keyStore.setEntry("myCA", entry, password);

    FileOutputStream fileName = new FileOutputStream(keyStoreFilePath);
    keyStore.store(fileName, keyStorePassword.toCharArray());

    System.out.println("CA Bundle genrated and saved to " + keyStoreFilePath);

  }

  public static void main(String[] args) {
      generateKeyPairAndCABundle("ca-keystore.jks", "changeit");
  }
}