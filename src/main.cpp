#include "../include/fs.h"
#include <iostream>
#include <cstring>

using namespace std;

void pause() {
    cout << "\nDevam etmek icin ENTER'a basin...";
    cin.ignore();
    cin.get();
}

int main() {
    int choice;
    string name, name2, backup;
    char buffer[512];

    do {
        cout << "\n========== SimpleFS Menu ==========\n"
             << " 1. Format disk\n"
             << " 2. Dosya olustur\n"
             << " 3. Dosyaya yaz\n"
             << " 4. Dosyadan oku\n"
             << " 5. Dosyalari listele\n"
             << " 6. Dosya sil\n"
             << " 7. Dosya var mi?\n"
             << " 8. Dosya boyutu\n"
             << " 9. Dosyaya veri ekle\n"
             << "10. Dosya ismini degistir\n"
             << "11. Dosya icerigini goster (cat)\n"
             << "12. Dosya icerigini kes/kucult (truncate)\n"
             << "13. Dosya kopyala\n"
             << "14. Dosya tasi (rename)\n"
             << "15. Disk butunlugunu kontrol et\n"
             << "16. Disk defragment et\n"
             << "17. Disk yedekle\n"
             << "18. Disk yedegini geri yukle\n"
             << "19. Iki dosya ayni mi? (diff)\n"
             << "20. Cikis\n"
             << "===================================\n"
             << "Seciminiz: ";
        cin >> choice;

        cin.ignore(); // yeni satır temizliği

        switch (choice) {
            case 1:
                fs_format();
                break;
            case 2:
                cout << "Dosya adi: ";
                getline(cin, name);
                if (!fs_create(name)) cout << "Dosya olusturulamadi!\n";
                break;
            case 3:
                cout << "Dosya adi: ";
                getline(cin, name);
                cout << "Yazilacak veri: ";
                cin.getline(buffer, sizeof(buffer));
                if (!fs_write(name, buffer, strlen(buffer)))
                    cout << "Yazma basarisiz!\n";
                break;
            case 4: {
                cout << "Dosya adi: ";
                getline(cin, name);
                int size = fs_size(name);
                if (size <= 0) {
                    cout << "Dosya bos veya bulunamadi.\n";
                    break;
                }
                char *buf = new char[size + 1];
                if (fs_read(name, 0, size, buf)) {
                    buf[size] = '\0';
                    cout << "İçerik:\n" << buf << "\n";
                } else {
                    cout << "Okuma basarisiz!\n";
                }
                delete[] buf;
                break;
            }
            case 5:
                fs_ls();
                break;
            case 6:
                cout << "Silinecek dosya adi: ";
                getline(cin, name);
                if (!fs_delete(name)) cout << "Silme basarisiz!\n";
                break;
            case 7:
                cout << "Dosya adi: ";
                getline(cin, name);
                cout << (fs_exists(name) ? "Dosya mevcut.\n" : "Dosya bulunamadi.\n");
                break;
            case 8:
                cout << "Dosya adi: ";
                getline(cin, name);
                cout << "Boyut: " << fs_size(name) << " byte\n";
                break;
            case 9:
                cout << "Dosya adi: ";
                getline(cin, name);
                cout << "Eklenecek veri: ";
                cin.getline(buffer, sizeof(buffer));
                if (!fs_append(name, buffer, strlen(buffer)))
                    cout << "Ekleme basarisiz!\n";
                break;
            case 10:
                cout << "Eski ad: ";
                getline(cin, name);
                cout << "Yeni ad: ";
                getline(cin, name2);
                if (!fs_rename(name, name2)) cout << "Yeniden adlandirma basarisiz!\n";
                break;
            case 11:
                cout << "Dosya adi: ";
                getline(cin, name);
                fs_cat(name);
                break;
            case 12: {
                cout << "Dosya adi: ";
                getline(cin, name);
                int new_size;
                cout << "Yeni boyut: ";
                cin >> new_size;
                if (!fs_truncate(name, new_size)) cout << "Kesme basarisiz!\n";
                break;
            }
            case 13:
                cout << "Kaynak dosya: ";
                getline(cin, name);
                cout << "Hedef dosya: ";
                getline(cin, name2);
                if (!fs_copy(name, name2)) cout << "Kopyalama basarisiz!\n";
                break;
            case 14:
                cout << "Taşınacak dosya adı: ";
                getline(cin, name);
                cout << "Yeni ad: ";
                getline(cin, name2);
                if (!fs_mv(name, name2)) cout << "Tasima basarisiz!\n";
                break;
            case 15:
                fs_check_integrity();
                break;
            case 16:
                fs_defragment();
                break;
            case 17:
                cout << "Yedek dosya adi (örn. disk.bak): ";
                getline(cin, backup);
                if (!fs_backup(backup)) cout << "Yedekleme basarisiz!\n";
                break;
            case 18:
                cout << "Yedegi geri yukle (örn. disk.bak): ";
                getline(cin, backup);
                if (!fs_restore(backup)) cout << "Geri yukleme basarisiz!\n";
                break;
            case 19:
                cout << "1. Dosya adı: ";
                getline(cin, name);
                cout << "2. Dosya adı: ";
                getline(cin, name2);
                cout << (fs_diff(name, name2) ? "Dosyalar ayni.\n" : "Dosyalar farkli.\n");
                break;
            case 20:
                cout << "Cikiliyor...\n";
                break;
            default:
                cout << "Gecersiz secim.\n";
        }

        if (choice != 20) pause();

    } while (choice != 20);

    return 0;
}


